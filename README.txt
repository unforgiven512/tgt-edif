		tgt-edif
		
		volodya-project.sourceforge.net


Getting tgt-edif to work:


1) Download iverilog source from usual place, put tgt-edif inside (or make a symlink).


2) Add the following lines to ../iverilog.conf

#
#
#
[-tedif]
<ivl>%B/ivl %[s-s%s] %[N-N%N] %[T-T%T] -tdll -fDLL=%B/edif.tgt -Fsynth -Fsyn-rules -Fcprop -Fnodangle -o%o -- -


3) Compile and install iverilog.

4) Compile and install tgt-edif


You can now produce edif files like this:

iverilog -tedif a.v -oa.edf


