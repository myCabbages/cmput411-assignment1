cmput411-assignment1
====================


This program displays a model of wavefront obj format, specified as a single argument in command line. The model and the camera may both be moved around, rotated etc. See the pdf description of the assignment for full features.

Development was done in eclipse, hence this folder actually can be imported into eclipse as a project really simply, and then built and run there. (Do not forget to specify which model to load in the command line arguments.)

Alternatively, the code can be compiled and then run with the following commands:
```
make
./modelviewer <path_to_obj_file>
```

To display debugging information, as well as the local model frame (with unit length colored lines), and the global frame turn the DEBUG constant on at the beginning of the main.cpp file.

--------------------
Developed by David Szepesvari. September 30, 2012.