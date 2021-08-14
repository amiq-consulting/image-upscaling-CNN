# PYNQ-Z1 files
To port the bicubic accelerator on PYNQ-Z1, you need to:
- put bicubic.bit and bicubic.hwh files in a folder named bicubic at the following path: '/home/xilinx/pynq/overlays/' (to transfer data from your computer to the board, you can use something like WinSCP or Putty);
- open PYNQ-Z1 Jupyter;
- upload the input_image.jpeg using files tab from the Jupyter Notebook;
- run all of the cells from 'BICUBIC INTERPOLATION - TB.ipynb' file;
- view the output_image.jpeg from the files tab.

Known issue:  
  Can not upscale a padded input image.
