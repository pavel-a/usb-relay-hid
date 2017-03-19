Please check https://sourceforge.net/projects/libusb/files/libusb-0.1%20%28LEGACY%29/ and download the latest from that project
Compiles at least with v0.1.12

The following should be used as reference of where to copy artifacts from: (you may need to manually inspect generated artifacts on libusb project)

cp --preserve=links libusb-0.1.12/.libs/*.so lib/usb/x86
cp --preserve=links libusb-0.1.12/.libs/*.so.* lib/usb/x86
cp libusb-0.1.12/.libs/*.la lib/usb/x86
