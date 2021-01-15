# OSES ASSIGNMENT ON EMBEEDED LINUX: Heart Rate Monitor

# - Steps to test the application

- Copy into the poky directory the layer meta-heartrate with

`git clone https://github.com/nicolabattaglino/meta-heartrate.git`

- Set up the environment   

`source oe-init-build-env build_qemuarm`

- Move to the folder /conf

- Add these lines to the local.conf file
```
IMAGE_INSTALL_append = " hbmod"
KERNEL_MODULE_AUTOLOAD += "hbmod"
IMAGE_INSTALL_append = " heartbeat"
```
- Go back in the /build_qemuarm  folder and add the layer with the command

`bitbake-layers add-layer ../meta-heartrate/`

- Now build with the command 

`bitbake core-image-minimal`

Open qemuarm and use `heartbeat` command to test the application

# - Heartbeat Application

The main function create a thread and performs an endless loop, calling the readAndConvert function every 20ms using the usleep() function, in order to respect the sampling frequency of 50Hz.
The ReadAndConvert function perform a read from the driver. It returns a value from the ppg array (declared in “data.h”), in the buffer, convert the value and then writes it in the pipe; at this point the pipe is full and so the thread reads from the pipe the value and stores it in the array. The pipe is useful to synchronise the main and the thread, because it’a blocking pipe.
When the application reaches 2048 reading, the thread performs a 2048-points FFT, compute the PSD and prints the bpm value.


