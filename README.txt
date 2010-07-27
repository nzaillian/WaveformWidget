libwaveformwidget
-----------------

This library provides a widget for the Qt framework capable of drawing the waveform of an audio file.  

DEPENDENCIES:
	this library depends on  
	(1) The Qt framework (tested with v. 4.6, but likely to work with earlier versions)
        (2) libsndfile (available from http://www.mega-nerd.com/libsndfile)

Full API documentation can be found at http://www.columbia.edu/~naz2106/WaveformWidget

The header and implementation files for this widget (class WaveformWidget) can be found in the src directory, as well as header and implementation files for a pair of classes leveraged by it.


BUILDING FROM SOURCE:

To build the widget as a dynamic library, simply invoke "qmake" and then "make" from within the "src" directory.

I have provided a shell script, "install.sh", that, when invoked with appropriate privileges, will copy the object files to your "/usr/lib" directory, and the header files to your "/usr/include" directory.  I have also provided an "uninstall.sh" script that will remove the files from these directories (useful if you modify the source files).

The "demo" directory contains a simple demonstration program that links to the dynamic library.  NOTE: this demo will not run if you have not yet built and installed the dynamic library!

The "devel" directory contains the same demo program, but instead of linking to the dynamic library, it compiles WaveformWidget and related classes from the source directory and links statically to them.


