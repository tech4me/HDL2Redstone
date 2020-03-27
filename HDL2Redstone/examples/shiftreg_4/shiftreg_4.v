module shift_register(clk, shift_in, shift_out);
input clk;
input shift_in;
output shift_out;
reg[3:0] bits;

assign shift_out = bits[3];

always @(posedge clk) begin
    bits <= bits << 1;
    bits[0] <= shift_in;
end

endmodule
