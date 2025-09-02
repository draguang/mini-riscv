module RegisterFile #(ADDR_WIDTH = 5, DATA_WIDTH = 32) (
  input clk,
  input rst,
  input [DATA_WIDTH-1:0] wdata,
  input [ADDR_WIDTH-1:0] waddr,
  input [ADDR_WIDTH-1:0] raddr1,
  input [ADDR_WIDTH-1:0] raddr2,
  input wen,
  input ren,
  output reg [DATA_WIDTH-1:0] rdata1,
  output reg [DATA_WIDTH-1:0] rdata2
);
  reg [DATA_WIDTH-1:0] rf [2**ADDR_WIDTH-1:0];
  export "DPI-C" function get_reg;
  function int get_reg(input int num);
    if(num>5'b11111)  begin 
      get_reg = 32'h11451419;
    end
    else  begin 
      get_reg = rf[num];
    end
  endfunction
  always @(posedge clk) begin
    if(rst) rf[0] <= 32'b0;
    else if (wen&&waddr!=0) rf[waddr] <= wdata;
  end
  assign rdata1 = (raddr1==0)?0:rf[raddr1];
  assign rdata2 = (raddr2==0)?0:rf[raddr2];
endmodule