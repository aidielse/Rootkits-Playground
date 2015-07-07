Updated: 07/06/2015

This is a repository for all of my rootkit/driver development experiments.

sources:

	Contains all of the files used by Visual Studio 2013 Professional.

binaries:



	Contains compiled executables and drivers.

	hello_world:

		Nothing too useful, this folder just contains the first few drivers I ever wrote.

	basic_driver:

		A pretty straightforward driver designed to be used as a skeleton for further development.

		Showcases driver basics like loading and unloading, handing different kinds of IRP's (including IOCTL's)
		etc.

	Skeleton_Rootkit:

		A more developed version of basic_driver. This is built to provide a more extensive Rootkit Building Framework.
		
		driverLauncher was extended and refined from hello_world. 
		driverLauncher can now be used to load drivers/rootkits into the kernel and then unload them as needed.

TODO: Add more documentation!