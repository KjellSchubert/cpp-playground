This tests some mem bandwidth assumptions we had made on a server that showed
certain CPU usage & mem bandwidth limitations.

See also http://www.admin-magazine.com/HPC/Articles/Finding-Memory-Bottlenecks-with-Stream

To gen vcproj:

gyp --depth=. -G msvs_version=2012 MemBandwidthTest.gyp