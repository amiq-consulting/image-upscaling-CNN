# PYNQ-Z1 files
To port the bicubic accelerator on PYNQ-Z1 and to upscale images, you need to:
- power up the PYNQ-Z1 board and connect the board directly to your computer using the Ethernet port and the Static IP - https://pynq.readthedocs.io/en/v2.6.1/getting_started/pynq_z1_setup.html;
- port the *bicubic.bit* and *the bicubic.hwh* files in a folder named *bicubic* on PYNQ-Z1 at the following path: */home/xilinx/pynq/overlays/* (to transfer data from your computer to the board, you can use something like WinSCP or Putty);
- navigate to http://192.168.2.99 to open PYNQ-Z1 Jupyter Notebooks;
- upload the *BICUBIC INTERPOLATION - TB.ipynb* and the *input_image.jpeg* files in PYNQ-Z1 Jupyter Notebooks;
- run all of the cells from *BICUBIC INTERPOLATION - TB.ipynb* file;
- view the output_image.jpeg.

Known issue:    
  Can not upscale images for *do_padding = 1* (see *BICUBIC INTERPOLATION - TB.ipynb* file).
