module multiplier_8(a, b, result);
	input [7:0] a, b;	
	output [15:0] result;

	assign result = a * b;

endmodule
