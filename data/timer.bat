@set /A _tic=%time:~0,2%*3600^
            +%time:~3,1%*10*60^
            +%time:~4,1%*60^
            +%time:~6,1%*10^
            +%time:~7,1% >nul

%1 --monte_carlo_samples=100000 --history_size=100000 --obfuscation_probability=.00000009 --vocabulary_size=50 --num_obfuscations=1 --simulate_obfuscations=1

@set /A _toc=%time:~0,2%*3600^
            +%time:~3,1%*10*60^
            +%time:~4,1%*60^
            +%time:~6,1%*10^
            +%time:~7,1% >nul

@set /A _elapsed=%_toc%-%_tic
@echo %_elapsed% seconds.