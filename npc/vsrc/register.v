module register #(DATA_WIDTH = 32)(
    input clk,
    input rst,
    input en,
    input [DATA_WIDTH-1:0]p,
    output reg [DATA_WIDTH-1:0]q
);
    always@(posedge clk) begin
        if(rst) q<=32'h00000000;
        else if(en)  q<=p;
    end
endmodule