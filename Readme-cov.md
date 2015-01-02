This here shows how to run lcov to get test coverage output with clang & gyp: 

```
# from http://stackoverflow.com/questions/23923040/generating-empty-gcda-files
lcov --no-external --zerocounters --directory out/
lcov --no-external --capture --initial --base-directory . --directory out/ --output-file coverage_base.info
run_test_here
lcov --no-external --capture --base-directory . --directory out/ --output-file coverage_test.info
lcov --add-tracefile coverage_base.info --add-tracefile coverage_test.info --output-file coverage_total.info
genhtml --output-directory coverage --title XCalibur-test-coverage coverage_total.info
```
