//
// Created by ZenderMario on 2023/4/23.
//

#ifndef Y86_64_SIMULATOR_UNIT_H

#include <iostream>
#include <stdexcept>

namespace Unit {
    static bool memErr = false;

    const size_t MAXMEM = UINT16_MAX;
    //the max number address units

    using RangeType = uint16_t;
    //to express  numbers we use( enough

    const RangeType numOP = 4;
    //the max num of operations ALU could execute

    const RangeType WORDBIT = 64;
    const RangeType BYTEBIT = 8;
    const RangeType ADDRBIT = 16;

    template<RangeType size>
    class Bit {
    private:
        bool bit[size] = {0};
    public:
        Bit() = default;

        Bit( std::initializer_list<bool> seq) {
            RangeType s = seq.size();
            std::initializer_list<bool>::iterator begin = seq.begin();
            for( RangeType i = 0; i < s && i < size; ++i) {
                bit[ size - s + i] = *begin++;
            }
        }

        void Set(const bool *pos) {
            for (RangeType i = 0; i < size; i++)
                bit[i] = pos[i];
        }

        RangeType Size() const{
            return size;
        }
        //return the size of whole bit array

        const bool * loca() const { return bit;}
        bool * loca() { return bit;}

        template< RangeType bits2>
        auto operator+( const Bit< bits2>& b2) const ->  Bit< size + bits2> const{
            Bit< size + bits2> ret;
            for( RangeType i = 0; i < size; ++i)
                ret[ i] = bit[ i];
            for( RangeType i = 0; i < bits2; ++i)
                ret[ size + i]  = b2[ i];

            return ret;
        }

        Bit& operator=(const Bit A) {
            for (RangeType i = 0; i < size; ++i)
                bit[i] = A.bit[i];
            return *this;
        }
        //used to give a value to Bit template

        const bool& operator[](RangeType pos) const{
            try {
                if (pos > size - 1)
                    throw std::out_of_range("Out of byte range!");
                return bit[pos];
            } catch (std::out_of_range &err) {
                memErr = true;
                std::cerr << err.what() << std::endl;
                return memErr;
            }
        }

        bool& operator[](RangeType pos) {
            try {
                if (pos > size - 1)
                    throw std::out_of_range("Out of byte range!");
                return bit[pos];
            } catch (std::out_of_range &err) {
                memErr = true;
                std::cerr << err.what() << std::endl;
                return memErr;
            }
        }
        //used to return a reference to some specific position

        void Print() const{
            for( RangeType i = 0; i < size; ++i) {
                if( i && !( i % 8))
                    std::cout << ' ';

                std::cout << bit[ i];
            }
            std::cout << std::endl;
        }
    };

    /*!
     * this part of code should be fixed to some proper formats, those I wrote here were a crack of s**t;
     */



    using Byte = Bit<BYTEBIT>;
    using Word = Bit<WORDBIT>;
    using Address = Bit<ADDRBIT>;

    /***********************************************************************/

    template< RangeType bits>
    class Decoder
    {
    public:
        uint64_t operator()( const Bit< bits>& b) {
            if( bits > ADDRBIT)
                throw std::range_error( "Unvalid Conversion!");

            RangeType ret = 0;
            RangeType s = b.Size();
            for( RangeType i = 0; i <s; ++i)
                ret += ( b[ i] << ( s - 1 - i));
            return ret;
        }
    };

    template< RangeType bits>
    class Coder
    {
    public:
        Bit< bits> operator()(uint64_t val) const{
            Bit< bits> ret;
            for( RangeType i = 0; i < bits; ++i)
                ret[ bits - 1 - i] = ( val >> i) & 0b1;
            return ret;
        }
    };
    const Coder<WORDBIT> wdCoder;

    /***********************************************************************/

    class ANDGate {
    public:
        Word operator()(const Word &A, const Word &B) const;
    };

    class ORGate {
    public:
        Word operator()(const Word &A, const Word &B) const;
    };

    class XORGate {
    public:
        Word operator()(const Word &A, const Word &B) const;
    };

    class NOTGate
    {
    public:
        Word operator()( const Word& A) const;
    };

    template< RangeType bits>
    class EQUGate
    {
    public:
        bool operator()(const Bit< bits> &A, const Bit< bits> &B) const{
            for( RangeType i = 0; i < bits; ++i)
                if( A[ i] != B[ i])
                    return false;
            return true;
        }};

    const ANDGate wdAND;
    const ORGate  wdOR;
    const XORGate wdXOR;
    const NOTGate wdNOT;
    const EQUGate< WORDBIT> wdEQU;

    template< RangeType bits>
    class Mutiplexor
    {
    private:
        Decoder< bits> decoder;
    public:
        Word operator()( const Bit< bits>& bit, const std::initializer_list<Word> &init) {
            return init[ decoder( bit)];
        }//choose a word within 2^b inputs
    };

    const Mutiplexor< 6> wdMutiPlexor;

    /***********************************************************************/


    struct AdderRes {
        bool carry = false;
        bool sum   = false;
    };

    class HlfAdder {
    public:
        HlfAdder() = default;

        AdderRes operator()(bool A, bool B) const;
    };

    const HlfAdder hlfADD;

    class FulAdder {
    public:
        FulAdder() = default;
        AdderRes operator()(bool A, bool B, bool C) const;

    };

    const FulAdder fulADD;

    template< RangeType bits>
    class Incrementer
    {
    public:
        Bit< bits> operator()(const Bit<bits> &bit, RangeType time = 1) {
            Bit< bits> tmp;
            tmp.Set( bit.loca());

            while( time > 0) {
                AdderRes res = hlfADD( 1, tmp[ bits - 1]);
                tmp[ bits - 1] = res.sum;

                for( int i = 1; i < bits; ++i) {
                    res = fulADD( 0, tmp[ bits - 1 - i], res.carry);
                    tmp[ bits - 1 - i] = res.sum;
                }

                --time;
            }

            return tmp;
        }
    };


    class Adder64
    {
    public:
        Adder64() = default;
        Word operator()(const Word &A, const Word &B) const;
    };

    const Adder64 wdADD;

    class Subber64{
    public:
        Subber64() = default;
        Word operator()( const Word& A, const Word& B) const;
    };

    const Subber64 wdSUB;

    class SHL64
    {
    public:
        Word & operator()( Word & word, RangeType bit) const;
    };

    class SHR64
    {
    public:
        Word & operator()( Word & word, RangeType bit) const;
    };

    class SAL64
    {
    public:
        Word & operator()( Word & word, RangeType bit) const;
    };

    class SAR64
    {
    private:
        Word & operator()( Word & word, RangeType bit) const;
    };

    template< RangeType bits>
    class ExtendTo64U
    {
    public:
        //this functor translates any bit to a word( by value)
        //and doesn't change its original value
        Word operator()(const Bit<bits>& bit) const{
            if( bits > 64)
                throw std::runtime_error( "the bits transformed to can't be greater than 64!\n");

            Word tmp;

            for( RangeType i = 0; i < bits; ++i) {
                tmp[ WORDBIT - 1 - i] = bit[ bits - 1 - i];
            }


            return tmp;
        }
    };

    template< RangeType bits>
    class ExtendTo64S
    {
    public:
        //this functor translates any bit to a word( by value)
        //and doesn't change its original value
        Word operator()(const Bit<bits>& bit) const{
            if( bits > 64)
                throw std::runtime_error( "the bits transformed to can't be greater than 64!\n");

            Word tmp;

            for( RangeType i = 0; i < bits; ++i) {
                tmp[ WORDBIT - 1 - i] = bit[ bits - 1 - i];
            }

            for( RangeType i = bits; i < WORDBIT; ++i) {
                tmp[ WORDBIT - 1 - i] = bit[ 0];
            }

            return tmp;
        }
    };

    const SAL64 wdSAL;
    const SAR64 wdSAR;
    const SHL64 wdSHL;
    const SHR64 wdSHR;

    /***********************************************************************/

    enum class MemoryStat{ OK, DISKERROR, INVALID};
    //record the working state of memory


    Byte Locate( const Word& word, RangeType n);
    //here we define a function to locate the nth byte in a word( to simply read a byte from memory



    class Memory
    {
    private:
        //because I couldn't simulate the physical memory like stack
        //I choose to design a class to simulate that process
        Byte mem[ MAXMEM];
        //to store bits in memory
        MemoryStat stat = MemoryStat::OK;
        //record recent memory state
        bool check = true;
        //I set check default value to true, if SEQ could detect wrong, you can turn it off.
        Decoder< 16> memDecoder;

        bool Check( Address pos, RangeType bias);
        //check if read/ write is valid, we set its default value to true to check,
        //but we can use SEQ to detect if there is wrong,
    public:
        Memory();

        Byte ReadB( Address pos);
        Byte WriteB( const Address& pos, const Byte& byte);
        //read/ write one byte

        Word ReadW( Address pos);
        Word WriteW( const Address& pos, const Word& word);
        //read/ write one word

        void Read( const Address& pos, bool * dst, Unit::RangeType size);
        //read size bytes to dst( make sure there is enough memory to write
        //otherwise MemoryStat::VALID will be set


        bool CheckMode() const { return check; };
        void SwitchCheck() { check = !check; }
        void MemReset() { stat = MemoryStat::OK; }
        bool Good() const { return stat == MemoryStat::OK;};
        bool Bad() const { return stat == MemoryStat::INVALID || stat == MemoryStat::DISKERROR; }
    };

    /***********************************************************************/


    using RegidLabel = Bit< 4>;
    using RegidValue = Word;

    class RegisterFile;

    struct Output
    {
        Word A;
        Word B;
    };
    //store what is read from registers( in decode phase

    const RegidLabel RRSP = { 0, 1, 0, 0};
    const RegidLabel NONE = { 1, 1, 1, 1};

    class Register
    {
    private:
        Decoder< 4> regDecoder;
        RegidValue reg[ 15];
        //0xf to indicate no register to be needed
        //0x0 to 0xf represents %rax to %r15 respectively
        friend class RegisterFile;
    public:
        RegidValue& operator[]( const RegidLabel& label);
        //return the proper register
    };

    class RegisterFile
    {
    private:
        RegidLabel dstE, dstM;
        //dest to write value, need a clock to enable
        //because of sequential order, we have to save those register label to write back later
        EQUGate< 4> regidEQU;

        Register& reg;
    public:
        explicit RegisterFile( Register& r) : reg( r) {};

        Output operator()( const RegidLabel& A, const RegidLabel& B, const RegidLabel& E, const RegidLabel& M);
        void operator()( const Word& valE, const Word& valM, bool cnd);
        //output data, cnd is sent from execute phase, decides whether to output( for cmov
        //because we send E first,  when interface E and M all need to output to the same register, consider of pop, we
        //say M is prior than E
    };

    /***********************************************************************/

    class PCIncrementer
    {
        ExtendTo64U< ADDRBIT> extender;

    public:
        Address operator()( const Address& adre, bool regiR, bool valC);
    };

    /***********************************************************************/

    class CC
    {
    private:
        bool ZF = true;
        //zero flag
        bool OF = false;
        //overflow flag
        bool SF = false;
        //signed flag

        public:
        enum class CondCode { L, LE, E, GE, G};
        //represent lower, lower-equal, equal, not-equal, greater-equal, greater

        bool L() { return ( SF ^ OF) || ZF;}
        bool LE() { return (SF ^ OF); }
        bool E() { return !ZF; }
        bool NE() { return ZF;}
        bool GE() { return !( SF ^ OF); }
        bool G() { return (!( SF ^ OF) && !ZF) ;}

        void SetZF() { ZF = true;}
        void SetOF() { OF = true;}
        void SetSF() { SF = true;}
        void Reset() { ZF = OF = SF = false;}

    };

    /***********************************************************************/

    using ALUCode = Bit< 2>;
    class ALU
    {
    private:

        ALUCode codeOP[ numOP] = {
                { 0, 0}, // ADD
                { 0, 1}, // SUB
                { 1, 0}, // AND
                { 1, 1}  // XOR
        };
        Decoder< 4> aluDecoder;

    public:
        Word operator()( const Word& A, const Word& B, const Bit< 4>& ifunc);
    };

    /***********************************************************************/

    using Code = Bit< 4>;
    const Bit< 4> consStat[ 4] =
    {
        { 0, 0, 0, 1}, //OK
        { 0, 0, 1, 0}, //HALT
        { 0, 1, 0, 0}, //ADR
        { 1, 0, 0, 0}, //INS
    };



    const Code CODE[ 13] = {
            { 0, 0, 0, 0}, //halt           1byte
            { 0, 0, 0, 1}, //nop            1byte
            { 0, 0, 1, 0}, //rrmov cmove    2bytes
            { 0, 0, 1, 1}, //irmov          10bytes
            { 0, 1, 0, 0}, //rmmov          10bytes
            { 0, 1, 0, 1}, //mrmov          10bytes
            { 0, 1, 1, 0}, //OPs            2bytes
            { 0, 1, 1, 1}, //jxxs           9bytes
            { 1, 0, 0, 0}, //call           9bytes
            { 1, 0, 0, 1}, //ret            1byte
            { 1, 0, 1, 0},//push           2bytes
            { 1, 0, 1, 1}, //pop           2bytes
            { 1, 1, 0, 0} //print          2bytes
    };

    const Code FUNC[ 3][ 7] = {
            {   { 0, 0, 0, 0},         //add
                    { 0, 0, 0, 1},        //sub
                    { 0, 0, 1, 0},        //and
                    { 0, 0, 1, 1}         //xor
            },//OP

            { { 0, 0, 0, 0}, //jmp
                    {0, 0, 0, 1},     //jl
                    { 0, 0, 1, 0},    //jle
                    {0, 0, 1, 1},     //je
                    { 0, 1, 0, 0},    //jne
                    { 0, 1, 0, 1},    //jge
                    {0, 1, 1, 0}      //jg
            },//jxx

            { { 0, 0, 0, 0},       //rrmov
                    {0, 0, 0, 1},      //cmovl
                    { 0, 0, 1, 0},     //cmovle
                    {0, 0, 1, 1},      //cmove
                    { 0, 1, 0, 0},     //cmovne
                    { 0, 1, 0, 1},     //cmovge
                    {0, 1, 1, 0}       //cmovg
            },//cmove

    };
}

#define Y86_64_SIMULATOR_UNIT_H

#endif //Y86_64_SIMULATOR_UNIT_H
