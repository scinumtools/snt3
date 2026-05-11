#include "snt/cli/puq_info.h"
#include "snt/cli/puq_eval.h"
#include "snt/cli/puq_convert.h"
#include "snt/cli/puq_list.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace snt;

TEST(PUQCommands, Info) {
  {
    cli::PUQInfo cmd("23*cm");
    
    testing::internal::CaptureStdout();
    cmd.execute();
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(output, "\nExpression:  23*cm\n\nUnit system: SI (International System of Units)\nResult:   23\nBase units:  cm\n\nDimensions:\n\nBase   Num*Mag                   Numerical                 Physical                  \n------ ------------------------- ------------------------- ------------------------- \nMGS    0.23                      0.01                      m                         \nMKS    0.23                      0.01                      m                         \nCGS    23                        1                         cm                        \n\nBase units:\n\nPrefix   Symbol   Exponent   Name                           Definition                     Dimensions MGS         Allowed prefixes       \n-------- -------- ---------- ------------------------------ ------------------------------ ---------------------- ---------------------- \nc        m        1          meter                          m                              0.01*m                 all                    \n\n");
  } 
  {
    cli::PUQInfo cmd("ft2*lb");
    cmd.argument_input_system("US");
    
    testing::internal::CaptureStdout();
    cmd.execute();
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(output, "\nExpression:  ft2*lb\n\nUnit system: US (United States customary units)\nResult:   1\nBase units:  ft2*lb\n\nDimensions:\n\nBase   Num*Mag                   Numerical                 Physical                  \n------ ------------------------- ------------------------- ------------------------- \nMGS    42.1401                   42.1401                   m2*g                      \nMKS    0.0421401                 0.0421401                 m2*kg                     \nCGS    4.21401e5                 4.21401e5                 cm2*g                     \n\nBase units:\n\nPrefix   Symbol   Exponent   Name                           Definition                     Dimensions MGS         Allowed prefixes       \n-------- -------- ---------- ------------------------------ ------------------------------ ---------------------- ---------------------- \n         lb       1          pound                          453.59237*g                    453.592*g                                     \n         ft       2          foot                           0.3048*m                       0.092903*m2                                   \n\n");
  } 
}

TEST(PUQCommands, Eval) {
  {
    cli::PUQEval cmd("23*cm + 3*m");

    testing::internal::CaptureStdout();
    cmd.execute();
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(output, "323*cm\n");
  }
  {
    cli::PUQEval cmd("23*cm + 3*m");
    cmd.argument_input_system("SI");
    cmd.argument_output_system("US");
    cmd.argument_output_units("ft");
    cmd.argument_output_quantity("l");
    
    testing::internal::CaptureStdout();
    cmd.execute();
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(output, "10.5971*ft\n");
  }
}

TEST(PUQCommands, Convert) {
  {
    cli::PUQConvert cmd("12*statA","A");
    cmd.argument_input_system("ESU");
    cmd.argument_output_system("SI");
    cmd.argument_output_quantity("I");
    
    testing::internal::CaptureStdout();
    cmd.execute();
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(output, "4.00277e-9*A\n");
  }
}

TEST(PUQList, SystemList) {
  {
    cli::PUQList cmd;
    
    testing::internal::CaptureStdout();
    cmd.execute();
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(output,"\nAvailable lists:\n\nprefix  unit prefixes\nbase    base units\nderiv   derived units\nlog     logarithmic units\ntemp    temperature units\nconst   constants\nquant   quantities\nsys     unit systems\n\n");
  }
  {
    cli::PUQList cmd("deriv");
    cmd.argument_system("AU");
    
    testing::internal::CaptureStdout();
    cmd.execute();
    std::string output = testing::internal::GetCapturedStdout();
    
    EXPECT_EQ(output,"Symbol    Name                   Result        Dimension                      Definition                Allowed prefixes       \n--------- ---------------------- ------------- ------------------------------ ------------------------- ---------------------- \nE_h       hartree                4.3597447276(40)e-15 m2*g*s-2                       {#hbar}2/({#m_e}*{a_0}2)                         \n");
  }
}
