@echo off

REM -[ ignore this ]---------------------
set BSIB_PSIB_TRIALS=10
set NUM_UNIQUE_CORPUS_TOKENS=10000
set MAX_LOAD_FACTOR=.01
set MIN_LOAD_FACTOR=.001
set NUM_TEST_QUERIES=30
set MIN_BLOCK_SIZE=2000
set MAX_BLOCK_SIZE=6000
set MIN_BINOMIAL=.075
set MAX_BINOMIAL=.2
set SEED=0
set JUNK_FRACTION=.3
set FREQ_ERROR=.3
REM -------------------------------------

set TRIALS=3
set TYPES=bsib psib psip psif
set SECRETS=secret
set RECALL_PRECISION=1
set MINDIST=1
set BM25=1
set FP_RATE=0.001

set MIN_LOCATION_UNCERTAINTY=256
set MAX_LOCATION_UNCERTAINTY=256
set INC_LOCATION_UNCERTAINTY=1

set MAX_NUM_DOC_TOKENS=4000
set MIN_NUM_DOC_TOKENS=4000
set INC_NUM_DOC_TOKENS=1

set MAX_NUM_CORPUS_DOCS=1000
set MIN_NUM_CORPUS_DOCS=1000
set INC_NUM_CORPUS_DOCS=1

set MAX_TERMS_PER_QUERY=2
set MIN_TERMS_PER_QUERY=1
set INC_TERMS_PER_QUERY=1

set MAX_TOKENS_PER_TERM=1
set MIN_TOKENS_PER_TERM=1
set INC_TOKENS_PER_TERM=1

set MAX_OBFUSCATIONS=0
set MIN_OBFUSCATIONS=0
set INC_OBFUSCATIONS=1

set /a NUM_CORPUS_DOCS=%MIN_NUM_CORPUS_DOCS%
:loop_start_num_corpus_docs
    set TOP_K=%NUM_CORPUS_DOCS%
    if %NUM_CORPUS_DOCS% gtr %MAX_NUM_CORPUS_DOCS% goto loop_end_num_corpus_docs
    
    set NUM_DOC_TOKENS=%MIN_NUM_DOC_TOKENS%
    :loop_start_num_doc_tokens
        if %NUM_DOC_TOKENS% gtr %MAX_NUM_DOC_TOKENS% goto loop_end_num_doc_tokens
        
        set /A number=%NUM_DOC_TOKENS%, last=2, sqrt=number/last
        :nextIter
           set /A last=(last+sqrt)/2, sqrt=number/last
        if %sqrt% lss %last% goto nextIter
        set /a NUM_UNIQUE_DOC_TOKENS=12*%sqrt%
        
        set LOCATION_UNCERTAINTY=%MIN_LOCATION_UNCERTAINTY%
        :loop_start_location_uncertainty
            if %LOCATION_UNCERTAINTY% gtr %MAX_LOCATION_UNCERTAINTY% goto loop_end_location_uncertainty
            
            set TERMS_PER_QUERY=%MIN_TERMS_PER_QUERY%
            :loop_start_terms_per_query
                if %TERMS_PER_QUERY% gtr %MAX_TERMS_PER_QUERY% goto loop_end_terms_per_query
                
                set TOKENS_PER_TERM=%MIN_TOKENS_PER_TERM%
                :loop_start_tokens_per_term
                    if %TOKENS_PER_TERM% gtr %MAX_TOKENS_PER_TERM% goto loop_end_tokens_per_term
                    
                    set OBFUSCATIONS=%MIN_OBFUSCATIONS%
                    :loop_start_obfuscations
                        if %OBFUSCATIONS% gtr %MAX_OBFUSCATIONS% goto loop_end_obfuscations
                        
                        call run_tests_helper.bat %1\%NUM_CORPUS_DOCS%_%NUM_DOC_TOKENS%_%LOCATION_UNCERTAINTY%_%TERMS_PER_QUERY%_%TOKENS_PER_TERM%_%OBFUSCATIONS%_%FP_RATE%

                        set /a OBFUSCATIONS+=%INC_OBFUSCATIONS%
                        goto loop_start_obfuscations
                        
                    :loop_end_obfuscations

                    set /a TOKENS_PER_TERM+=INC_TOKENS_PER_TERM%
                    goto loop_start_tokens_per_term

                :loop_end_tokens_per_term

                set /a TERMS_PER_QUERY+=1
                goto loop_start_terms_per_query
                
            :loop_end_terms_per_query

            set /a LOCATION_UNCERTAINTY+=%INC_LOCATION_UNCERTAINTY%
            goto loop_start_location_uncertainty
            
        :loop_end_location_uncertainty

        set /a NUM_DOC_TOKENS+=%INC_NUM_DOC_TOKENS%
        goto loop_start_num_doc_tokens
        
    :loop_end_num_doc_tokens

    set /a NUM_CORPUS_DOCS+=%INC_NUM_CORPUS_DOCS%
    goto loop_start_num_corpus_docs
    
:loop_end_num_corpus_docs