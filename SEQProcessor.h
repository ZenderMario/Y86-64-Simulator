//
// Created by ZenderMario on 2023/4/22.
//

#ifndef Y86_64_SIMULATOR_CODE_H

#include <string>
#include "Unit.h"
#include <fstream>

namespace SEQ
{
    /*
     * Those two classes are designed to deal with mechanical code
     * it will call a function accepting a string argument that contains code.
     * In this implementation, I will cover the whole process that Y86-64 that holds,
     * including basic instructions, function call, pipeline, and such that.
     * There might be Parallel Code for future, but these are my all mind.
     * And I am sure this will be of significance in the simulator I will build for a whole Computer.
     */


    /***********************************************************************/

    using Unit::Bit;

    using Unit::Word;
    using Unit::Byte;
    using Unit::RegidLabel;
    using Unit::RangeType;
    using Unit::Address;
    using Code = Unit::Bit< 4>;

    using Unit::Register;
    using Unit::RegisterFile;
    using Unit::CC;
    using Unit::Memory;
    using Unit::MemoryStat;
    using Unit::ALU;
    using Unit::Decoder;
    using Unit::Incrementer;

    using Unit::WORDBIT;
    using Unit::BYTEBIT;
    using Unit::ADDRBIT;

    using Unit::RRSP; //constant for register %rsp
    using Unit::NONE; //represents no register is needed

    //we still leave space for more various instructions,
    //but it's enough for my simulator

    class SeqProcessor
    {
    private:

        Address PC;
        //always points to next instruction
        //because we set its original value to 0x0, we have to make sure the program starts at address 0x0
        Incrementer< Unit::ADDRBIT> pcIncrementer;
        //increase PC by one
        struct {
            Code icode;
            Code ifunc;
        }IR;
        //store the instruction what is executed in this tk
        Bit< 4> Stat = { 0, 0, 0, 1};
        //set Stat to OK
        //record the state which the processor runs in
        Register reg;
        //store general purpose registers
        RegisterFile regFile;
        //to read or write from/ to registers
        Unit::Output outputReg;
        //store what read from registers( in decode phase
        CC condCode;
        //recode recent result state ALU calculated
        Memory mem;
        //record the state of memory working
        ALU aluUnit;
        //make simple calculations
        Unit::Decoder< 4> icodeDecoder;

        struct CU
        {
            bool needRegis = false;
            bool needCons  = false;
            bool instrInvalid = false;
            bool imemErr = false;
            bool dmemErr = false;
            bool cnd = false;
        }ctrlUnit;
        //Control Unit

        bool isPrint = false;

        void Fetch( const Bit< 10 * BYTEBIT>& ins, Word& valP, RegidLabel& A,
                    RegidLabel& B, Word& s);
        //split the Code field read to two parts
        void Decode( const RegidLabel& A, const RegidLabel& B);
        //apply ID to decode the instruction
        //use two four-bits read to tell register file
        Word Execute( const Word& cons);
        //apply ALU to make some operations
        void Access( const Word& valE, const Word& valA, const Word& valP, Word& valM);
        //access the memory
        void Write( const Word& valE, const Word& valM);
        //write back to register
        //remember Register has two inputs, and pop instruction has two register to write
        //we declare input M is prior to E
        void Update( const Word& valC, const Word& valM, const Word& valP);
        //update PC
        void Print();
        //prints out all information



        Word ConvertW( const std::string& s);
        Address ConvertA( const std::string& s);
        std::string Element( RangeType countL, const std::string &str, const std::string flag1, uint8_t bias1, const std::string flag2,
                             uint8_t bias2, bool rFind1 = false, bool rFind2 = false);
        //construc a string from an existing str, the position depends on beg and end
        void Translates();
        //converts my asm code to machine code
        void ErrorMessage( const std::string& str, RangeType line);


    public:
        SeqProcessor() : regFile( reg) {
            Stat = Unit::consStat[ 0];
            //set Stat to OK
            reg[ Unit::RRSP] = {
                    0, 0, 0, 0,  0, 0, 0, 0,
                    0, 0, 0, 0,  0, 0, 0, 0,
                    0, 0, 0, 0,  0, 0, 0, 0,
                    0, 0, 0, 0,  0, 0, 0, 0,
                    0, 0, 0, 0,  0, 0, 0, 0,
                    0, 0, 0, 0,  0, 0, 0, 0,
                    0, 0, 0, 0,  0, 0, 0, 0,
                    0, 0, 0, 0,  0, 0, 0, 0,
            };
            //the top of stack locates in 4096bytes away from
            //the beginning of program
            //we just use the last two bytes of %rsp,
            //because we have limited memory

        }
        void Run();
    };



}

#define Y86_64_SIMULATOR_CODE_H

#endif //Y86_64_SIMULATOR_CODE_H
