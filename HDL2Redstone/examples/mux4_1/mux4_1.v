module multiplexer4_1 ( a ,b ,c ,d ,x ,y ,dout );

output dout;
input a, b, c, d, x, y;

assign dout = (a & (~x) & (~y)) |
     (b & (~x) & (y)) |
     (c & x & (~y)) |
     (d & x & y);
endmodule
