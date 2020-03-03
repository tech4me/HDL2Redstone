module dff (input D, input clk, output reg Q);
	always @ (posedge clk)
		if (clk) begin
			Q<=D;
		end
endmodule
