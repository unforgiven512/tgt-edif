		tgt-edif
		
		volodya-project.sourceforge.net


Getting tgt-edif to work:


1) Download iverilog source from usual place, put tgt-edif inside (or make a symlink).


2) Add the following lines to ../iverilog.conf

#
#
#
[-tedif-atmel-at40k]
<ivl>%B/ivl %[s-s%s] %[N-N%N] %[T-T%T] -tdll -fDLL=%B/edif.tgt -Fsynth -Fsyn-rules -Fcprop -Fnodangle -farch=atmel-at40k -o%o -- -
#
#
#
[-tedif-xilinx-spartan]
<ivl>%B/ivl %[s-s%s] %[N-N%N] %[T-T%T] -tdll -fDLL=%B/edif.tgt -Fsynth -Fsyn-rules -Fcprop -Fnodangle -farch=xilinx-spartan -o%o -- -


3) Compile and install iverilog.

4) Compile and install tgt-edif


You can now produce edif files like this:

iverilog -tedif-atmel-at40k a.v -oa.edf


Getting tgt-edif to work with unsupported FPGA/place and route tool:

   You need to creat a *.inc file for your fpga. Take a look at 
   atmel_at40k.inc and xilinx_spartan.inc for examples.
   
   You can learn the correct syntax for intrinsic (i.e. known to P&R tool)
   modules by examining libraries that come with your P&R tool install and
   by studying EDIF output produced by some other synthesizer. Two sample
   verilog files addc.v and sample.v are included to help.
   
   
