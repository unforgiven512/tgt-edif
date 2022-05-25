/* sample module to exercise edif output features */

/*
 * before running thru your synthesizer modify this and/or project settings so that it:
 *   a) compiles on your synthesizer
 *   b) *_i? correspond to input pads
 *   c) *_o? correspond to output pads
 *   d) *_c? correspond to clock pads
 *
 * also try instatiating BUFIF (highimpendance when 0 on select input) and BUF (just a buffer) modules.
 */

module main(
	and_i1,
	and_i2,
	and_o1,
	or_i1,
	or_i2,
	or_o1,
	xor_i1,
	xor_i2,
	xor_o1,
	nor_i1,
	nor_i2,
	nor_o1,
	nand_i1,
	nand_i2,
	nand_o1,
	xnor_i1,
	xnor_i2,
	xnor_o1,
	bufif_i1,
	bufif_i2,
	bufif_i3,
	bufif_o1,
	inv_i1,
	inv_o1,
	mux_i1,
	mux_i2,
	mux_i3,
	mux_o1,
	add_i1,
	add_i2,
	add_o1,
	dff_i1,
	dff_c1,
	dff_o1,
	zero_o1,
	one_o1,
	buf_i1,
	buf_o1
);

	input and_i1;
	input and_i2;
	output and_o1;
	input or_i1;
	input or_i2;
	output or_o1;
	input xor_i1;
	input xor_i2;
	output xor_o1;
	input nor_i1;
	input nor_i2;
	output nor_o1;
	input nand_i1;
	input nand_i2;
	output nand_o1;
	input xnor_i1;
	input xnor_i2;
	output xnor_o1;
	input bufif_i1;
	input bufif_i2;
	input bufif_i3;

	reg bufif_data;
	wire bufif_r;

	output bufif_o1;
	input inv_i1;
	output inv_o1;
	input mux_i1;
	input mux_i2;
	input mux_i3;
	output mux_o1;
	input [7:0] add_i1;
	input [7:0] add_i2;
	output [7:0] add_o1;
	input dff_i1;
	input dff_c1;

	reg dff_data;

	output dff_o1;
	output zero_o1;
	output one_o1;
	input buf_i1;
	output buf_o1;

	assign and_o1 = and_i1 & and_i2;
	assign or_o1 = or_i1 | or_i2;
	assign xor_o1 = xor_i1 ^ xor_i2;
	assign nor_o1 = ~(nor_i1 | nor_i2);
	assign nand_o1 = ~(nand_i1 & nand_i2);
	assign xnor_o1 = ~(xnor_i1 ^ xnor_i2);

	bufif1(
		bufif_r,
		bufif_i2,
		bufif_i3
	);

	always @(posedge bufif_i1)
		bufif_data = bufif_r;

	assign bufif_o1 = bufif_data;
	assign inv_o1 = ~inv_i1;
	assign mux_o1 = mux_i1?mux_i2:mux_i3;
	assign add_o1 = add_i1+add_i2;

	always @(posedge dff_c1)
		begin
			dff_data = dff_i1;
		end

	assign dff_o1 = dff_data;

	assign zero_o1 = 1'b0;
	assign one_o1 = 1'b1;

	buf buf_cell(
		buf_o1,
		buf_i1
	);

endmodule
