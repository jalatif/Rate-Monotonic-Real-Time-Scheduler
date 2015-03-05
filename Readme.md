
1. 
    Building Modules
        make modules

    Building UserApp
        make app

    Building Both
        make all

2. install module
    sudo insmod mp2.ko

3. using userapp
    ./userapp <period_in_secs> <computation_time_for_the_realtime_app>
    example:
        ./userapp 5 1
        ./userapp 5 2

4. remove module
    sudo rmmod mp2.ko
