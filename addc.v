

module addc(A, B, CIN, COUT, Q);

input A, B, CIN;
output COUT, Q;
assign Q=A ^ B ^ CIN;
assign COUT= (A & B) | (A & CIN) | (B & CIN);
endmodule
