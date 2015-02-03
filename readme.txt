CrashDoctor
--------
This program tries to bypass the faulting instruction in a crashing
program. It is implemented as a JIT debugger and activated by windows
whenever a program crashes.


Compile
-----------
- Use Visual Studio 2013 to compile the software.
- First download IHULIB library from:
  https://github.com/IntellectualHeaven/ihulib/releases/download/v1.0/ihulib_v_1_0.zip
- Extract this to crashdoctor/extrn/ihulib (preserve folder paths)
- You should have files like ihulib/bin/x64/Debug/ihulib.lib etc.
- Open crashdoctor.sln in make folder and build.


Install
-------
Just run CrashDoctor32.msi or CrashDoctor64.msi depending upon your
OS version.

