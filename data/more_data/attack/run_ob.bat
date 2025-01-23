@echo off

set PROG_NAME=
set OUTPUT=
set MIN_OBS=
set MAX_OBS=
set OBS_INC=
set MIN_VOCAB=
set MAX_VOCAB=
set VOCAB_INC=
set TRIALS=
set MONTE_CARLO_SAMPLES=
set HISTORY_SIZE=

set PROG_NAME=%0
set OUTPUT=%1

set MIN_OBS=%2
set MAX_OBS=%3
set OBS_INC=%4

set MIN_VOCAB=%5
set MAX_VOCAB=%6
set VOCAB_INC=%7

set TRIALS=%8
set MONTE_CARLO_SAMPLES=%9

if "%1"=="" goto error
if "%2"=="" goto error
if "%3"=="" goto error
if "%4"=="" goto error
if "%5"=="" goto error
if "%6"=="" goto error
if "%7"=="" goto error
if "%8"=="" goto error
if "%9"=="" goto error

shift
shift

set HISTORY_SIZE=%8
set OB_PROB=%9

if "%8"=="" goto error
if "%9"=="" goto error

echo OUTPUT: %OUTPUT%
echo MIN_OBS: %MIN_OBS%
echo MAX_OBS: %MAX_OBS%
echo OBS_INC: %OB_INC%
echo MIN_VOCAB: %MIN_VOCAB%
echo MAX_VOCAB: %MAX_VOCAB%
echo VOCAB_INC: %VOCAB_INC%
echo TRIALS: %TRIALS%
echo MONTE_CARLO_SAMPLES: %MONTE_CARLO_SAMPLES%
echo HISTORY_SIZE: %HISTORY_SIZE%
echo OB_PROB: %OB_PROB%
echo.

set OBS=%MIN_OBS%
:loop_start_ob
    if %OBS% gtr %MAX_OBS% goto loop_end_ob
    set VOCAB=%MIN_VOCAB%
    :loop_start_vocab
        if %VOCAB% gtr %MAX_VOCAB% goto loop_end_vocab

	AttackSimSSE2 ^
		--trials=%TRIALS% ^
		--history_size=%HISTORY_SIZE% ^
		--monte_carlo_samples=%MONTE_CARLO_SAMPLES% ^
		--vocabulary_size=%VOCAB% ^
		--num_secrets=1 ^
		--num_obfuscations=%OBS% ^
		--obfuscation_probability=%OB_PROB% ^
		--simulate_obfuscations=1 >> %OUTPUT%

        set /a VOCAB+=%VOCAB_INC%
	goto loop_start_vocab
    :loop_end_vocab
    
    set /a OBS+=%OBS_INC%
    goto loop_start_ob

:loop_end_ob

goto done

:error
echo usage: %PROG_NAME% [OUTPUT]
echo     [MIN_OBS] [MAX_OBS] [OB_INC]
echo     [MIN_VOCAB] [MAX_VOCAB] [VOCAB_INC]
echo     [TRIALS] [MONTE_CARLO_SAMPLES] [HISTORY_SIZE]
echo.
echo example: %PROG_NAME% results 1 50 50 50 1 10 1000000 100000
echo.
echo errors:

if "%OUTPUT%"=="" 		        echo    must specify OUTPUT
if "%MIN_OBS%"=="" 		    echo    must specify MIN_OBS
if "%MAX_OBS%"=="" 		    echo    must specify MAX_OBS
if "%OBS_INC%"=="" 		    echo    must specify OBS_INC
if "%MIN_VOCAB%"=="" 		    echo    must specify MIN_VOCAB
if "%MAX_VOCAB%"=="" 		    echo    must specify MAX_VOCAB
if "%VOCAB_INC%"=="" 		    echo    must specify VOCAB_INC
if "%TRIALS%"=="" 		        echo    must specify TRIALS
if "%MONTE_CARLO_SAMPLES%"=="" 	echo    must specify MONTE_CARLO_SAMPLES
if "%HISTORY_SIZE%"==""		    echo    must specify HISTORY_SIZE
echo.

:done
