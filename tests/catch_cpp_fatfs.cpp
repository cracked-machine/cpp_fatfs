
// MIT License

// Copyright (c) 2022 Chris Sutton

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <catch2/catch_all.hpp>
#include <iostream>

// TLC5955 device datasheet:
// https://www.ti.com/lit/ds/symlink/tlc5955.pdf


// TEST_CASE("Testing TLC5955 common register", "[tlc5955]")
// {

//     tlc5955::tlc5955_tester leds_tester;

    
//     // @brief Testing tlc5955::Driver::flush_common_register()
//     SECTION("Test flush command")
//     {
//         // set all the bits to 1
//         std::for_each (leds_tester.data_begin(), leds_tester.data_end(), [](auto &byte){
//             byte = 0xFF;
//         });

//         // check they are set to 1
//         std::for_each (leds_tester.data_begin(), leds_tester.data_end(), [](auto &byte){
//             REQUIRE(byte == 0xFF);
//         });

//         // run the flush command
//         leds_tester.reset();

//         // check the bits are all cleared
//         std::for_each (leds_tester.data_begin(), leds_tester.data_end(), [](auto &byte){
//             REQUIRE(byte == 0x00);
//         });
//     }

//     // @brief Testing tlc5955::Driver::set_control_bit
//     SECTION("Latch bit test")
//     {
//         // Latch       
//         // bits      =
//         // Bytes     [
//         //          #0          
//         // latch bit test
        
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::latch) == 0b00000000);   

//         leds_tester.set_control_bit(true);

//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::latch) == 0b10000000);      // 128
//     }

//     // @brief Testing tlc5955::Driver::set_ctrl_cmd_bits()
//     SECTION("Control bits test")
//     {
//         // control byte test

//         // Ctrl      10010110  
//         // bits      [======]
//         // Bytes     ======][
//         //             #0    #1

//         leds_tester.set_control_bit(true);
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::latch) == 0b10000000);      // 128

//         leds_tester.set_ctrl_cmd_bits();
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::ctrl_cmd) == 0b1100'1011);      // 203

//         // clear the previous test
//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::ctrl_cmd) == 0b0000'0000);         

//     }

//     // Testing tlc5955::Driver::set_padding_bits()
//     SECTION("Padding bits test")
//     {
//         // set all bytes to 0xFF
//         std::for_each (leds_tester.data_begin(), leds_tester.data_end(), [](auto &byte){
//             byte = 0xFF;
//         });

//         // padding bits test - bytes 1-48 should be empty 
//         leds_tester.set_padding_bits();

//         // first byte should have MSB set e.g. 0b10000000
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::padding) == 0x80);

//         // next 47 bytes should be 0x00
//         for (
//             uint8_t byte_idx = (leds_tester.byte_offsets::padding + 1); 
//             byte_idx < (leds_tester.byte_offsets::function); 
//             byte_idx++
//         )
//         {
//             REQUIRE(+leds_tester.get_common_reg_at(byte_idx) == 0x00);
//         }
        
//         // last byte should have partially cleared e.g. 0b00000011
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::function) == 0x03);
//     }

//     // Testing tlc5955::Driver::set_function_data()
//     SECTION("Function bits test")
//     {
//         // function bits test   
//         // bits      [===]
//         //           =][==
//         // Bytes   #49  #50

//         leds_tester.set_function_data(true, false, false, false, false);
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::function) == 0b00000010);  // 2
        
//         leds_tester.set_function_data(true, true, false, false, false);
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::function) == 0b00000011);  // 3
                        
//         leds_tester.set_function_data(true, true, true, false, false);
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::function + 1) == 0b10000000);  // 128
        
//         leds_tester.set_function_data(true, true, true, true, false);
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::function + 1) == 0b11000000);  // 192

//         leds_tester.set_function_data(true, true, true, true, true);
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::function + 1) == 0b11100000);  // 224

//         // clear the previous test
//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::function) == 0b00000000);  
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::function + 1) == 0b00000000); 

//     }
    
//     // @brief Testing tlc5955::Driver::set_bc_data()
//     SECTION("Brightness Control bits test")
//     {
//         // BC bits test
//         // BC         blue   green   red
//         // bits      [=====][=====][=====]
//         // bits      ====][======][======]
//         // Bytes     #50    #51      #52    

//         const std::bitset<leds_tester.m_bc_data_size> preset_bc_test_pattern_0x55 {0b1010101};
//         const std::bitset<leds_tester.m_bc_data_size> preset_bc_test_pattern_0x2A {0b0101010};        

//         // set the bit pattern
//         leds_tester.set_bc_data(
//             preset_bc_test_pattern_0x55, 
//             preset_bc_test_pattern_0x2A, 
//             preset_bc_test_pattern_0x55);            
      
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::brightness_control) == 0b00010101);  
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::brightness_control + 1) == 0b01010101); 
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::brightness_control + 2) == 0b01010101);    

//         // clear the previous test
//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::brightness_control) == 0b00000000);  
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::brightness_control + 1) == 0b00000000); 
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::brightness_control + 2) == 0b00000000);    

//         // set the inverse bit pattern
//         leds_tester.set_bc_data(
//             preset_bc_test_pattern_0x2A, 
//             preset_bc_test_pattern_0x55, 
//             preset_bc_test_pattern_0x2A);            
      
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::brightness_control) == 0b00001010);  
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::brightness_control + 2) == 0b10101010); 
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::brightness_control + 2) == 0b10101010);  

//         // clear the previous test
//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::brightness_control) == 0b00000000);  
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::brightness_control + 1) == 0b00000000); 
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::brightness_control + 2) == 0b00000000);        

//     }

//     // @brief  Testing tlc5955::Driver::set_mc_data()
//     SECTION("Max Current bit test")
//     {
//         // MC         B  G  R
//         // bits      [=][=][=]      // 9 bits
//         // bits      [======][
//         // Bytes       #53    #54     

//         const std::bitset<leds_tester.m_mc_data_size> preset_mc_test_pattern_0x5 {0b101};
//         const std::bitset<leds_tester.m_mc_data_size> preset_mc_test_pattern_0x2 {0b010};
 
//         // set bit pattern
//         leds_tester.set_mc_data(
//             preset_mc_test_pattern_0x5, preset_mc_test_pattern_0x2, preset_mc_test_pattern_0x5);

//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::max_current) == 0b10101010);                        // 170
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::max_current + 1) == 0b10000000);                        // 128
        
//         // clear the previous test
//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::max_current) == 0x00);                              // 0
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::max_current + 1) == 0x00);                              // 0

//         // set bit pattern
//         leds_tester.set_mc_data(
//             preset_mc_test_pattern_0x2, preset_mc_test_pattern_0x5, preset_mc_test_pattern_0x2);

//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::max_current) == 0b01010101);                        // 85
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::max_current + 1) == 0b00000000);                        // 0

//         // clear the previous test
//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::max_current) == 0x00);                              // 0
//         REQUIRE(+leds_tester.get_common_reg_at(leds_tester.byte_offsets::max_current + 1) == 0x00);                              // 0

//     }
// }

// // @brief Testing tlc5955::Driver::set_gs_data()
// TEST_CASE("Greyscale bit tests", "[tlc5955]")
// {
//     tlc5955::tlc5955_tester leds_tester;

//     // @brief Test Preset: all bits of one colour on 
//     const std::bitset<leds_tester.m_gs_data_size> preset_gs_test_on {0xFFFF};
//     // @brief Test Preset: all bits of one colour off
//     const std::bitset<leds_tester.m_gs_data_size> preset_gs_test_off {0};

//     // @brief Test Preset: alternating bit pattern
//     const std::bitset<leds_tester.m_gs_data_size> preset_gs_test_pattern_0xAAAA {0b1010101010101010};
//     // @brief Test Preset: inverse alternating bit pattern
//     const std::bitset<leds_tester.m_gs_data_size> preset_gs_test_pattern_0x5555 {0b0101010101010101};

//     SECTION("Invalid LED index")
//     {
//         REQUIRE_FALSE(leds_tester.set_gs_data(16, preset_gs_test_off, preset_gs_test_off, preset_gs_test_off));
//     }

//     // @brief Set the bit pattern via the API and read it back directly from byte array
//     SECTION("GS bit test 0xAAAA pattern")
//     {
//         // Example Byte Mapping
//         // ROW #1
//         // GS            B15             G15             R15              B14             G14             R14            B13             G13             R13   
//         // bits    0[==============][==============][==============][==============][==============][==============][==============][==============][==============]
//         // Bytes   [======][======][======][======][======][======][======][======][======][======][======][======][======][======][======][======][======][======][
//         //            #0      #1      #2      #3      #4      #5      #6      #7      #8      #9     #10     #11     #12     #13      #14     #15     #16     #17
//         for (uint16_t led_idx = 0; led_idx < 16; led_idx++)
//         {
//             // set the bit pattern
//             leds_tester.flush_common_register();
//             REQUIRE(leds_tester.set_gs_data(
//                 led_idx, 
//                 preset_gs_test_pattern_0xAAAA, 
//                 preset_gs_test_pattern_0xAAAA, 
//                 preset_gs_test_pattern_0xAAAA));

//             // GS data sections are grouped in bytes of 6
//             uint16_t byte_idx = led_idx * 6;
//             REQUIRE(+leds_tester.get_common_reg_at(byte_idx)    == 0b01010101); // byte 0
//             REQUIRE(+leds_tester.get_common_reg_at(++byte_idx)  == 0b01010101); // byte 1
//             REQUIRE(+leds_tester.get_common_reg_at(++byte_idx)  == 0b01010101); // byte 2
//             REQUIRE(+leds_tester.get_common_reg_at(++byte_idx)  == 0b01010101); // byte 3
//             REQUIRE(+leds_tester.get_common_reg_at(++byte_idx)  == 0b01010101); // byte 4
//             REQUIRE(+leds_tester.get_common_reg_at(++byte_idx)  == 0b01010101); // byte 5
//             REQUIRE(+leds_tester.get_common_reg_at(++byte_idx)  == 0b00000000); // byte 6
//         }
//     }

//     // @brief Set the inverse bit pattern via the API and read it back directly from byte array
//     SECTION("GS bit test 0x5555 pattern")
//     {
//         // Example Byte Mapping
//         // ROW #1
//         // GS            B15             G15             R15              B14             G14             R14            B13             G13             R13   
//         // bits    0[==============][==============][==============][==============][==============][==============][==============][==============][==============]
//         // Bytes   [======][======][======][======][======][======][======][======][======][======][======][======][======][======][======][======][======][======][
//         //            #0      #1      #2      #3      #4      #5      #6      #7      #8      #9     #10     #11     #12     #13      #14     #15     #16     #17
        
//         for (uint16_t led_idx = 0; led_idx < 16; led_idx++)
//         {
//             // set the inverse bit pattern
//             leds_tester.flush_common_register();
//             REQUIRE(leds_tester.set_gs_data(
//                 led_idx, 
//                 preset_gs_test_pattern_0x5555, 
//                 preset_gs_test_pattern_0x5555, 
//                 preset_gs_test_pattern_0x5555));

//             // GS data sections are grouped in bytes of 6
//             uint16_t byte_idx = led_idx * 6;
//             REQUIRE(+leds_tester.get_common_reg_at(byte_idx)    == 0b00101010); // byte 0
//             REQUIRE(+leds_tester.get_common_reg_at(++byte_idx)  == 0b10101010); // byte 1
//             REQUIRE(+leds_tester.get_common_reg_at(++byte_idx)  == 0b10101010); // byte 2
//             REQUIRE(+leds_tester.get_common_reg_at(++byte_idx)  == 0b10101010); // byte 3
//             REQUIRE(+leds_tester.get_common_reg_at(++byte_idx)  == 0b10101010); // byte 4
//             REQUIRE(+leds_tester.get_common_reg_at(++byte_idx)  == 0b10101010); // byte 5
//             REQUIRE(+leds_tester.get_common_reg_at(++byte_idx)  == 0b10000000); // byte 6
//         }
//     }
// }


// // @brief Testing tlc5955::Driver::set_dc_data()
// TEST_CASE("Dot Correction bit tests", "[tlc5955]")
// {
//     tlc5955::tlc5955_tester leds_tester;

//     // @brief Test Preset: all bits of one colour on 
//     const std::bitset<leds_tester.m_dc_data_size> preset_dc_test_on {127};
//     // @brief Test Preset: all bits of one colour off
//     const std::bitset<leds_tester.m_dc_data_size> preset_dc_test_off {0};
//     // @brief Test Preset: alternating bits for positional testing
//     const std::bitset<leds_tester.m_dc_data_size> preset_dc_test_pattern_0x55 {0b1010101};
//     const std::bitset<leds_tester.m_dc_data_size> preset_dc_test_pattern_0x2A {0b0101010};


//     // @brief Test out of bounds
//     SECTION("Invalid LED Index")
//     {
//         REQUIRE_FALSE(leds_tester.set_dc_data(16, preset_dc_test_on, preset_dc_test_on, preset_dc_test_on));
//     }

//     SECTION("LED15 Dot Correction bits test")
//     {

//         // Common Register-to-Byte array mapping for DC (dot correction) data
        
//         // ROW #1
//         // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ======][======][======][======][======][======][======][======][======][======][====
//         //          #54     #55     #56     #57      #58     #59    #60     #61     #62     #63     #64
 
//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             15, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(54) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(55) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(56) == 0b01010100);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(54) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(55) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(56) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             15, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(54) == 0b00101010);
//         REQUIRE(+leds_tester.get_common_reg_at(55) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(56) == 0b10101000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(54) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(55) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(56) == 0x00);

//     }

//     SECTION("LED14 Dot Correction bits test")
//     {

//         // Common Register-to-Byte array mapping for DC (dot correction) data
        
//         // ROW #1
//         // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ======][======][======][======][======][======][======][======][======][======][====
//         //          #54     #55     #56     #57      #58     #59    #60     #61     #62     #63     #64

//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             14, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(56) == 0b00000010);
//         REQUIRE(+leds_tester.get_common_reg_at(57) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(58) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(59) == 0b10100000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(56) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(57) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(58) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(59) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             14, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(56) == 0b00000001);
//         REQUIRE(+leds_tester.get_common_reg_at(57) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(58) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(59) == 0b01000000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(56) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(57) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(58) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(59) == 0x00);
        
//     }
//     SECTION("LED13 Dot Correction bits test")
//     {

//         // Common Register-to-Byte array mapping for DC (dot correction) data
        
//         // ROW #1
//         // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ======][======][======][======][======][======][======][======][======][======][====
//         //          #54     #55     #56     #57      #58     #59    #60     #61     #62     #63     #64

//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             13, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(59) == 0b00010101);
//         REQUIRE(+leds_tester.get_common_reg_at(60) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(61) == 0b01010101);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(59) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(60) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(61) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             13, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(59) == 0b00001010);
//         REQUIRE(+leds_tester.get_common_reg_at(60) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(61) == 0b10101010);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(59) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(60) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(61) == 0x00);
        
//     }

//     SECTION("LED12 Dot Correction bits test")
//     {

//         // Common Register-to-Byte array mapping for DC (dot correction) data
        
//         // ROW #1
//         // DC        B15    G15    R15    B14    G14    R14    B13    G13    R13    B12    G12    R12
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ======][======][======][======][======][======][======][======][======][======][====
//         //          #54     #55     #56     #57      #58     #59    #60     #61     #62     #63     #64

//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             12, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(62) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(62) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(64) == 0b10101000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(62) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(63) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(64) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             12, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(62) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(63) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(64) == 0b01010000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(62) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(63) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(64) == 0x00);        
//     }

//     SECTION("LED11 Dot Correction bits test")
//     {

//         // ROW #2
//         // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
//         //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74
 
//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             11, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(64) == 0b00000101);
//         REQUIRE(+leds_tester.get_common_reg_at(65) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(66) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(67) == 0b01000000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(64) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(65) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(66) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(67) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             11, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(64) == 0b00000010);
//         REQUIRE(+leds_tester.get_common_reg_at(65) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(66) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(67) == 0b10000000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(64) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(65) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(66) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(67) == 0x00);
        
//     }

//     SECTION("LED10 Dot Correction bits test")
//     {

//         // ROW #2
//         // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
//         //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74

//          // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             10, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(67) == 0b00101010);
//         REQUIRE(+leds_tester.get_common_reg_at(68) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(69) == 0b10101010);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(67) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(68) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(69) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             10, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(67) == 0b00010101);
//         REQUIRE(+leds_tester.get_common_reg_at(68) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(69) == 0b01010100);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(67) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(68) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(69) == 0x00);

//     }

//     SECTION("LED9 Dot Correction bits test")
//     {

//         // ROW #2
//         // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
//         //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74

//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             9, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(69) == 0b00000001);
//         REQUIRE(+leds_tester.get_common_reg_at(70) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(71) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(72) == 0b01010000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(69) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(70) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(71) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(72) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             9, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(69) == 0b00000000);
//         REQUIRE(+leds_tester.get_common_reg_at(70) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(71) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(72) == 0b10100000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(69) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(70) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(71) == 0x00);    
//         REQUIRE(+leds_tester.get_common_reg_at(72) == 0x00);    
//     }

//     SECTION("LED8 Dot Correction bits test")
//     {

//         // ROW #2
//         // DC        B11    G11    R11    B10    G10    R10    B9     G9     R9     B8     G8     R8
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
//         //        #64   #65      #66    #67     #68     #69     #70     #71     #72     #73     #74
 
//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             8, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(72) == 0b00001010);
//         REQUIRE(+leds_tester.get_common_reg_at(73) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(74) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(75) == 0b10000000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(72) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(73) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(74) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(75) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             8, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(72) == 0b00000101);
//         REQUIRE(+leds_tester.get_common_reg_at(73) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(74) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(75) == 0b00000000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(72) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(73) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(74) == 0x00);    
//         REQUIRE(+leds_tester.get_common_reg_at(75) == 0x00);  
//     }

//     SECTION("LED7 Dot Correction bits test")
//     {

//         // ROW #3
//         // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ======][======][======][======][======][======][======][======][======][======][====
//         //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             7, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(75) == 0b01010101);  // B7
//         REQUIRE(+leds_tester.get_common_reg_at(76) == 0b01010101);  // G7
//         REQUIRE(+leds_tester.get_common_reg_at(77) == 0b01010100);  // G7 + R7

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(75) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(76) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(77) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             7, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(75) == 0b00101010);
//         REQUIRE(+leds_tester.get_common_reg_at(76) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(77) == 0b10101000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(75) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(76) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(77) == 0x00);    
//     }

//     SECTION("LED6 Dot Correction bits test")
//     {

//         // ROW #3
//         // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ======][======][======][======][======][======][======][======][======][======][====
//         //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             6, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(77) == 0b00000010);  // B6
//         REQUIRE(+leds_tester.get_common_reg_at(78) == 0b10101010);  // B6 + G6
//         REQUIRE(+leds_tester.get_common_reg_at(79) == 0b10101010);  // G6 + R6
//         REQUIRE(+leds_tester.get_common_reg_at(80) == 0b10100000);  // R6

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(77) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(78) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(79) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(80) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             6, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(77) == 0b00000001);
//         REQUIRE(+leds_tester.get_common_reg_at(78) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(79) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(80) == 0b01000000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(77) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(78) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(79) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(80) == 0x00);  
        
//     }

//     SECTION("LED5 Dot Correction bits test")
//     {

//         // ROW #3
//         // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ======][======][======][======][======][======][======][======][======][======][====
//         //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             5, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(80) == 0b00010101);
//         REQUIRE(+leds_tester.get_common_reg_at(81) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(82) == 0b01010101);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(80) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(81) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(82) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             5, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(80) == 0b00001010);
//         REQUIRE(+leds_tester.get_common_reg_at(81) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(82) == 0b10101010);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(80) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(81) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(82) == 0x00);
//     }

//     SECTION("LED4 Dot Correction bits test")
//     {

//         // ROW #3
//         // DC        B7      G7     R7    B6     G6     R6     B5     G5     R5     B4     G4     R4
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ======][======][======][======][======][======][======][======][======][======][====
//         //          #75     #76     #77     #78     #79     #80     #81     #82     #83     #84     #85

//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             4, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(83) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(84) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(85) == 0b10101000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(83) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(84) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(85) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             4, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(83) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(84) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(85) == 0b01010000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(83) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(84) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(85) == 0x00);
//     }

//     SECTION("LED3 Dot Correction bits test")
//     {

//         // ROW #4
//         // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
//         //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             3, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(85) == 0b00000101);
//         REQUIRE(+leds_tester.get_common_reg_at(86) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(87) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(88) == 0b01000000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(85) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(86) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(87) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(88) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             3, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(85) == 0b00000010);
//         REQUIRE(+leds_tester.get_common_reg_at(86) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(87) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(88) == 0b10000000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(85) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(86) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(87) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(88) == 0x00);
//     }

//     SECTION("LED2 Dot Correction bits test")
//     {

//         // ROW #4
//         // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
//         //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             2, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(88) == 0b00101010);
//         REQUIRE(+leds_tester.get_common_reg_at(89) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(90) == 0b10101010);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(88) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(89) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(90) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             2, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(88) == 0b00010101);
//         REQUIRE(+leds_tester.get_common_reg_at(89) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(90) == 0b01010100);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(88) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(89) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(90) == 0x00);

//     }    

//     SECTION("LED1 Dot Correction bits test")
//     {

//         // ROW #4
//         // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
//         //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             1, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(90) == 0b00000001);
//         REQUIRE(+leds_tester.get_common_reg_at(91) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(92) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(93) == 0b01010000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(90) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(91) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(92) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(93) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             1, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(90) == 0b00000000);
//         REQUIRE(+leds_tester.get_common_reg_at(91) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(92) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(93) == 0b10100000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(90) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(91) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(92) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(93) == 0x00);
//     }        

//     SECTION("LED0 Dot Correction bits test")
//     {

//         // ROW #4
//         // DC        B3     G3     R3      B2     G2    R2      B1    G1     R1     B0     G0     R0
//         // bits    [=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====][=====]
//         // Bytes   ==][======][======][======][======][======][======][======][======][======][======][
//         //        #85   #86     #87      #88    #89     #90     #91     #92     #93     #94     #95   #96    

//         // set the bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             0, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55));
//         REQUIRE(+leds_tester.get_common_reg_at(93) == 0b00001010);
//         REQUIRE(+leds_tester.get_common_reg_at(94) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(95) == 0b10101010);
//         REQUIRE(+leds_tester.get_common_reg_at(96) == 0b10000000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(93) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(94) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(95) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(96) == 0x00);

//         // set the inverse bit pattern
//         REQUIRE(leds_tester.set_dc_data(
//             0, preset_dc_test_pattern_0x2A, preset_dc_test_pattern_0x55, preset_dc_test_pattern_0x2A));
//         REQUIRE(+leds_tester.get_common_reg_at(93) == 0b00000101);
//         REQUIRE(+leds_tester.get_common_reg_at(94) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(95) == 0b01010101);
//         REQUIRE(+leds_tester.get_common_reg_at(96) == 0b00000000);

//         leds_tester.flush_common_register();
//         REQUIRE(+leds_tester.get_common_reg_at(93) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(94) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(95) == 0x00);
//         REQUIRE(+leds_tester.get_common_reg_at(96) == 0x00);
//     }        


// }