#include <gtest/gtest.h>
#include <iostream>

TEST(ADDTEST,ADDTEST_TRUE)
{
    int num = 1;
    EXPECT_EQ(num,1);
}


int main(int argc, char **argv) {  
 std::cout << "Running main() from gtest_main.cc\n";  
 testing::InitGoogleTest(&argc, argv);  
 return RUN_ALL_TESTS();  
} 
