@echo off

if "%1"=="" goto error

mkdir %1
set trial=0

:loop_start
set /a trial+=1

if %trial% gtr %TRIALS% goto loop_end

mkdir %1\%trial%

echo ----------------------------------------
echo running trial %trial% out of %TRIALS%
echo ----------------------------------------

echo making corpus tokens
random_unigram_model ^
    --unique_tokens=%NUM_UNIQUE_CORPUS_TOKENS% ^
    --model=1 ^
    --outfile=%1\%trial%\corpus_tokens ^
    --min_token_size=1 ^
    --max_token_size=12 ^
    --seed=%SEED% 

echo making corpus
docgen.exe ^
    --min_unique_tokens=%NUM_UNIQUE_DOC_TOKENS% ^
    --max_unique_tokens=%NUM_UNIQUE_DOC_TOKENS% ^
    --min_tokens=%NUM_DOC_TOKENS% ^
    --max_tokens=%NUM_DOC_TOKENS% ^
    --model=%1\%trial%\corpus_tokens ^
    --num_docs=%NUM_CORPUS_DOCS% ^
    --output_dir=%1\%trial%\corpus ^
    --seed=%SEED%

echo making query set
random_query_gen ^
    --num_queries=%NUM_TEST_QUERIES% ^
    --min_terms=%TERMS_PER_QUERY% ^
    --max_terms=%TERMS_PER_QUERY% ^
    --min_tokens=%TOKENS_PER_TERM% ^
    --max_tokens=%TOKENS_PER_TERM% ^
    --outfile=%1\%trial%\qs ^
    --min_token_size=1 ^
    --max_token_size=12 ^
    --seed=%SEED%

echo seeding corpus
query_seeder.exe ^
    --in_dir=%1\%trial%\corpus ^
    --out_dir=%1\%trial%\corpus_seeded ^
    --query_set=%1\%trial%\qs ^
    --max_load_factor=%MAX_LOAD_FACTOR% ^
    --min_load_factor=%MIN_LOAD_FACTOR% ^
    --min_block_size=%MIN_BLOCK_SIZE% ^
    --max_block_size=%MAX_BLOCK_SIZE% ^
    --min_binomial=%MIN_BINOMIAL% ^
    --max_binomial=%MAX_BINOMIAL% ^
    --seed=%SEED%

echo making secure indexes
simaker.exe ^
    --types=canonical %TYPES% ^
    --indir=%1\%trial%\corpus_seeded ^
    --false_positive_rate=%FP_RATE% ^
    --location_uncertainty=%LOCATION_UNCERTAINTY% ^
    --secrets=%SECRETS% ^
    --outdir=%1\%trial%\corpus_indexed ^
    --junk_fraction=%JUNK_FRACTION% ^
    --freq_error=%FREQ_ERROR% ^
    --psib.algo=0 ^
    --psif.algo=0 ^
    --psip.algo=0

echo making query tests
qt.exe ^
    --recall_precision=%RECALL_PRECISION% ^
    --mindist=%MINDIST% ^
    --bm25=%BM25% ^
    --top_k=%TOP_K% ^
    --trials=%BSIB_PSIB_TRIALS% ^
    --indir=%1\%trial%\corpus_indexed ^
    --query_set=%1\%trial%\qs ^
    --secrets=%SECRETS% ^
    --obfuscations=%OBFUSCATIONS% ^
    --types=%TYPES% > %1\%trial%\results

move maker_*.log %1\%trial%\corpus_indexed > nul
move maker.log %1\%trial%\corpus_indexed > nul

Tabulator -i %1\%trial% --min_tokens_per_doc=%NUM_DOC_TOKENS% --max_tokens_per_doc=%NUM_DOC_TOKENS% >> results.csv

move %1\%trial%\corpus_indexed\*.log %1\%trial%\ > nul
rd /S /Q %1\%trial%\corpus_indexed > nul
rd /S /Q %1\%trial%\corpus > nul
rd /S /Q %1\%trial%\corpus_seeded > nul
del %1\%trial%\corpus_tokens > nul

echo ID %1\%trial% >> experiment.log
echo NUM_UNIQUE_CORPUS_TOKENS %NUM_UNIQUE_CORPUS_TOKENS% >> experiment.log
echo NUM_DOC_TOKENS %NUM_DOC_TOKENS% >> experiment.log
echo NUM_UNIQUE_DOC_TOKENS %NUM_UNIQUE_DOC_TOKENS% >> experiment.log
echo NUM_CORPUS_DOCS %NUM_CORPUS_DOCS% >> experiment.log
echo TERMS_PER_QUERY %TERMS_PER_QUERY% >> experiment.log
echo TOKENS_PER_TERM %TOKENS_PER_TERM% >> experiment.log
echo NUM_TEST_QUERIES %NUM_TEST_QUERIES% >> experiment.log
echo MAX_LOAD_FACTOR %MAX_LOAD_FACTOR% >> experiment.log
echo MIN_LOAD_FACTOR %MIN_LOAD_FACTOR% >> experiment.log
echo MIN_BLOCK_SIZE %MIN_BLOCK_SIZE% >> experiment.log
echo MAX_BLOCK_SIZE %MAX_BLOCK_SIZE% >> experiment.log
echo MIN_BINOMIAL %MIN_BINOMIAL% >> experiment.log
echo MAX_BINOMIAL %MAX_BINOMIAL% >> experiment.log
echo FP_RATE %FP_RATE% >> experiment.log
echo LOCATION_UNCERTAINTY %LOCATION_UNCERTAINTY% >> experiment.log
echo SECRETS %SECRETS% >> experiment.log
echo TYPES %TYPES% >> experiment.log
echo RECALL_PRECISION %RECALL_PRECISION% >> experiment.log
echo MINDIST %MINDIST% >> experiment.log
echo BM25 %BM25% >> experiment.log
echo BSIB_PSIB_TRIALS %BSIB_PSIB_TRIALS% >> experiment.log
echo OBFUSCATIONS %OBFUSCATIONS% >> experiment.log
echo JUNK_FRACTION %JUNK_FRACTION% >> experiment.log
echo TOP_K %TOP_K% >> experiment.log
echo. >> experiment.log

echo NUM_UNIQUE_CORPUS_TOKENS %NUM_UNIQUE_CORPUS_TOKENS% >> %1\%trial%\experiment.log
echo NUM_DOC_TOKENS %NUM_DOC_TOKENS% >> %1\%trial%\experiment.log
echo NUM_UNIQUE_DOC_TOKENS %NUM_UNIQUE_DOC_TOKENS% >> %1\%trial%\experiment.log
echo NUM_CORPUS_DOCS %NUM_CORPUS_DOCS% >> %1\%trial%\experiment.log
echo TERMS_PER_QUERY %TERMS_PER_QUERY% >> %1\%trial%\experiment.log
echo TOKENS_PER_TERM %TOKENS_PER_TERM% >> %1\%trial%\experiment.log
echo NUM_TEST_QUERIES %NUM_TEST_QUERIES% >> %1\%trial%\experiment.log
echo MAX_LOAD_FACTOR %MAX_LOAD_FACTOR% >> %1\%trial%\experiment.log
echo MIN_LOAD_FACTOR %MIN_LOAD_FACTOR% >> %1\%trial%\experiment.log
echo MIN_BLOCK_SIZE %MIN_BLOCK_SIZE% >> %1\%trial%\experiment.log
echo MAX_BLOCK_SIZE %MAX_BLOCK_SIZE% >> %1\%trial%\experiment.log
echo MIN_BINOMIAL %MIN_BINOMIAL% >> %1\%trial%\experiment.log
echo MAX_BINOMIAL %MAX_BINOMIAL% >> %1\%trial%\experiment.log
echo FP_RATE %FP_RATE% >> %1\%trial%\experiment.log
echo LOCATION_UNCERTAINTY %LOCATION_UNCERTAINTY% >> %1\%trial%\experiment.log
echo SECRETS %SECRETS% >> %1\%trial%\experiment.log
echo TYPES %TYPES% >> %1\%trial%\experiment.log
echo RECALL_PRECISION %RECALL_PRECISION% >> %1\%trial%\experiment.log
echo MINDIST %MINDIST% >> %1\%trial%\experiment.log
echo BM25 %BM25% >> %1\%trial%\experiment.log
echo BSIB_PSIB_TRIALS %BSIB_PSIB_TRIALS% >> %1\%trial%\experiment.log
echo OBFUSCATIONS %OBFUSCATIONS% >> %1\%trial%\experiment.log
echo JUNK_FRACTION %JUNK_FRACTION% >> %1\%trial%\experiment.log
echo TOP_K %TOP_K% >> %1\%trial%\experiment.log

goto loop_start

:error

echo error, no output directory specified

:loop_end
