/* sample module to exercise edif output features */

/* before running thru your synthesizer modify this and/or project
   settings so that it 
     a) compiles on your synthesizer
     b) *_i? correspond to input pads
     c) *_o? correspond to output pads
     d) *_c? correspond to clock pads
     
     also try instatiating BUFIF (highimpendance when 0 on select input)
     and BUF (just a buffer) modules.
     
*/

module main(and_i1, and_i2, and_o1,
	    or_i1, or_i2, or_o1,
	    xor_i1, xor_i2, xor_o1,
	    inv_i1, inv_o1,
	    mux_i1, mux_i2, mux_i3, mux_o1,
	    add_i1, add_i2, add_o1,
	    dff_i1, dff_c1, dff_o1
	);
input and_i1,and_i2;
output and_o1;
input or_i1, or_i2;
output or_o1;
input xor_i1, xor_i2;
output xor_o1;
input inv_i1;
output inv_o1;
input mux_i1, mux_i2, mux_i3;
output mux_o1;
input add_i1, add_i2;
output add_o1;
input dff_i1, dff_c1;
reg dff_data;
output dff_o1;

assign and_o1=and_i1 & and_i2;
assign or_o1=or_i1 | or_i2;
assign xor_o1=xor_i1 ^ xor_i2;
assign inv_o1=~inv_i1;
assign mux_o1=mux_i1?mux_i2:mux_i3;
assign add_o1=add_i1+add_i2;
always @(posedge dff_c1)
	begin
	  dff_data=dff_i1;
	end
assign dff_o1=dff_data;

endmodule
