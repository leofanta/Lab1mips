//
//  main.cpp
//  Lab1mips
//
//  Created by Jen Liu on 10/19/16.
//  Copyright © 2016 Jen Liu. All rights reserved.
//

#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define ADDU 1
#define SUBU 3
#define AND 4
#define OR  5
#define NOR 7
#define MemSize 65536 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.


class RF
{
public:
    bitset<32> ReadData1, ReadData2;
    RF()
    {
        Registers.resize(32);
        Registers[0] = bitset<32> (0);
    }
    
    void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
    {
        // implement the funciton by you.
        //Read data.
        ReadData1 = Registers[RdReg1.to_ulong()];
        ReadData2 = Registers[RdReg2.to_ulong()];
        
        //Write data if Write is enabled.
        if (WrtEnable[0]) {
            Registers[WrtReg.to_ulong()] = WrtData;
        }
    }
    
    void OutputRF()
    {
        ofstream rfout;
        rfout.open("RFresult.txt",std::ios_base::app);
        if (rfout.is_open())
        {
            rfout<<"A state of RF:"<<endl;
            for (int j = 0; j<32; j++)
            {
                rfout << Registers[j]<<endl;
            }
            
        }
        else cout<<"Unable to open file";
        rfout.close();
        
    }
private:
    vector<bitset<32> >Registers;
    
};

class ALU
{
public:
    bitset<32> ALUresult;
    bitset<32> ALUOperation (bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
    {
        // implement the ALU operations by you.
        switch (ALUOP.to_ulong()) {
            case ADDU:
                ALUresult = bitset<32>(oprand1.to_ulong() + oprand2.to_ulong());
                break;
            case SUBU:
                ALUresult = bitset<32>(oprand1.to_ulong() - oprand2.to_ulong());
                break;
            case AND:
                ALUresult = oprand1 & oprand2;
                break;
            case OR:
                ALUresult = oprand1 | oprand2;
                break;
            case NOR:
                ALUresult = ~(oprand1 | oprand2);
                break;
            default:
                break;
        }
        return ALUresult;
    }
};

class INSMem
{
public:
    bitset<32> Instruction;
    INSMem()
    {       IMem.resize(MemSize);
        ifstream imem;
        string line;
        int i=0;
        imem.open("imem.txt");
        if (imem.is_open())
        {
            while (getline(imem,line))
            {
                line = line.substr(0,8);
                IMem[i] = bitset<8>(line);
                i++;
            }
            
        }
        else cout<<"Unable to open file\n";
        imem.close();
        
    }
    
    bitset<32> ReadMemory (bitset<32> ReadAddress)
    {
        //to avoid overflow
        ReadAddress = ReadAddress & bitset<32>(0x0000ffff);
        
        // implement by you. (Read the byte at the ReadAddress and the following three byte).
        int i = 0;
        string Ins;
        
        while (i<4) {
            Ins.append(IMem[i+ReadAddress.to_ulong()].to_string());
            i++;
        }
        Instruction = bitset<32>(Ins);//cast to bitset
        return Instruction;
    }
    
private:
    vector<bitset<8> > IMem;
    
};

class DataMem
{
public:
    bitset<32> readdata;
    DataMem()
    {
        DMem.resize(MemSize);
        ifstream dmem;
        string line;
        int i=0;
        dmem.open("dmem.txt");
        if (dmem.is_open())
        {
            while (getline(dmem,line))
            {
                line = line.substr(0,8);
                DMem[i] = bitset<8>(line);
                i++;
            }
        }
        else cout<<"Unable to open file\n";
        dmem.close();
        
    }
    bitset<32> MemoryAccess (bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem)
    {
        //to avoid overflow
        Address = Address & bitset<32>(0x0000ffff);
        
        // implement by you.
        //read data.
        long addr = Address.to_ulong();
        string s;
        if(readmem[0]){
            
            for(int i =0; i<4;i++){
                s.append(DMem[addr + i].to_string());
            }
            readdata = bitset<32>(s);
        }else if(writemem[0] ){
            s = WriteData.to_string();
            
            for(int i =0; i< 4;i++){
                DMem[addr+i] =  bitset<8>(s.substr(i*8,8));
            }
        }
        return readdata;
   }
    
    void OutputDataMem()
    {
        ofstream dmemout;
        dmemout.open("dmemresult.txt");
        if (dmemout.is_open())
        {
            for (int j = 0; j< 1000; j++)
            {
                dmemout << DMem[j]<<endl;
            }
            
        }
        else cout<<"Unable to open file";
        dmemout.close();
        
    }
    
private:
    vector<bitset<8> > DMem;
    
};

string seg(bitset<32> instruction, int start, int length){
    string seg;
    seg = instruction.to_string().substr(start, length);
    return seg;
}

bitset<32> jumpAddr(bitset<32> instruction, bitset<32> nextPC){
    string jumpAddr = seg(nextPC, 0, 4).append(seg(instruction, 6, 26));
    jumpAddr.append("00");
    return bitset<32>(jumpAddr);
}

//recheck numbers in this function!
bitset<32> branchAddr(bitset<32> instruction){
    bool negative = !instruction[15];
    string branchAddr = negative? "00000000000000":"11111111111111";
    branchAddr.append(seg(instruction, 16, 16));
    branchAddr.append("00");
    return bitset<32>(branchAddr);
}

//recheck numbers in this function!
bitset<32> signExt(bitset<32> instruction){
    bool negative = !instruction[15];
    string signExtImm = negative? "0000000000000000":"1111111111111111";
    signExtImm.append(seg(instruction, 16, 16));
    return bitset<32>(signExtImm);
}

int main(int argc, const char * argv[])
{
    RF myRF;
    ALU myALU;
    INSMem myInsMem;
    DataMem myDataMem;
    
    bitset<32> nextPC;//where to initialize this parameter?
    
    while (1)
    {
        
        // Fetch instruction. myInsMem.
        bitset<32> instruction = myInsMem.ReadMemory(nextPC);
        
        // If current insturciton is "11111111111111111111111111111111", then break;
        if (instruction == 0xffffffff) {
            break;
        }
        
        // generate decoder varients, nextPC is calculated later
        bitset<6> opCode = bitset<6>(seg(instruction, 0, 6));
        bitset<5> rs = bitset<5>(seg(instruction, 6, 5));
        bitset<5> rt = bitset<5>(seg(instruction, 11, 5));
        bitset<5> rd = bitset<5>(seg(instruction, 16, 5));
        bitset<3> ALUOP;
        bitset<32> signExtImm = signExt(instruction);// to be implemented!
        
        bool isLoad = (opCode == 0x23);
        bool isStore = (opCode == 0x2b);
        bool isIType = (opCode != 0x00) && (opCode != 0x02);
        bool WrtEnable;
        bool isJType = (opCode == 0x02);
        bool isBranch = (opCode == 0x04);

        //decide ALUOP
        if (isLoad || isStore) //for I type instruction, addu.
            ALUOP = 1;
        else if (opCode == 0) //for R type instruction
            ALUOP = bitset<3>(seg(instruction, 29, 3));
        else                       //for J type instruction. It would be 2 for j.
            ALUOP =bitset<3>(seg(instruction, 3, 3));
        WrtEnable = !(isStore || isJType || isBranch);
        
        // decode(Read RF)
        myRF.ReadWrite(rs, rt, 0, 0, 0);
        
        // Execute
        bitset<32> ALUresult = myALU.ALUOperation(ALUOP, myRF.ReadData1, isIType?signExtImm:myRF.ReadData2);
        
        // Read/Write Mem
        bitset<32> readdata = myDataMem.MemoryAccess(ALUresult, myRF.ReadData2, isLoad, isStore);
        
        // Write back to RF
        myRF.ReadWrite(0, 0, isIType?rt:rd, isLoad?readdata:ALUresult, WrtEnable);//isLoad? readdata:ALUresult.
        
        //decide nextPC
        if (isBranch && (myRF.ReadData2 == myRF.ReadData1))
            nextPC = nextPC.to_ulong() + 4 + branchAddr(instruction).to_ulong();
        else if (isJType)
            nextPC = nextPC.to_ulong() + 4 + jumpAddr(instruction, nextPC).to_ulong();
        else
            nextPC = nextPC.to_ulong() + 4;
        myRF.OutputRF(); // dump RF;

    }
    myDataMem.OutputDataMem(); // dump data mem
    
    return 0;
}
