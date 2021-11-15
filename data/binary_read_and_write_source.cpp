#include <iostream>
#include <fstream>

bool verifyFileOpen(std::ifstream & source, std::ofstream & target);

int main(int argc, char** argv)
{
	std::cout << std::endl;

	if(argc != 4)
	{
		std::cout << "Error: Incorrect number of arguments" << std::endl;
		return 0;
	}

	if((char)argv[1][0] == (char)"r"[0]) // Check if reading is the specified operation
	{
		char c;
		int tempNum,
			num;
		bool byteType = true;

		// Open Files
		std::ifstream source(argv[2], std::ios::binary);
		std::ofstream target(argv[3], std::ios::trunc);

		// Verify that the files are open
		if(!verifyFileOpen(source, target))
		{
			return 0;
		}

		// Convert binary numbers to ASCII and write to the target.
		while(source.get(c))
		{
			tempNum = (int)c;
			if(tempNum < 0)
				tempNum += 256;

			if(byteType)
				num = tempNum * 256;
			else
				target << num + tempNum << " ";
			byteType = !byteType;
		}

		// Close Files
		source.close();
		target.close();
	}
	else if((char)argv[1][0] == (char)"w"[0]) // Check if writing is the specified operation
	{
		int tempNum,
			numHigh,
			numLow;

		// Open Files
		std::ifstream source(argv[2]);
		std::ofstream target(argv[3], std::ios::binary | std::ios::trunc);

		// Verify that the files are open
		if(!verifyFileOpen(source, target))
		{
			return 0;
		}

		// Convert ASCII numbers to binary and write to the target.
		while(source >> numLow)
		{	
			numHigh = 0;
			for(int i = 1; i <= 128; i += i)
		    {  
		       if((numLow / (32768 / i)) >= 1)
		       {
		          numHigh += 128 / i;
		          numLow -= 32768 / i;
		       }
		    }
			target << (char)numHigh << (char)numLow;
		}

		// Close Files
		source.close();
		target.close();
	}
	else
	{
		std::cout << "Error: Unknown operation '" << argv[1] << "'" << std::endl;
		return 0;
	}

	std::cout << "Done!" << std::endl;
	return 1;
}

bool verifyFileOpen(std::ifstream & source, std::ofstream & target)
{
	if(!source.is_open())
	{
		std::cout << "Error: The source file failed to open" << std::endl;
		return 0;
	}

	if(!target.is_open())
	{
		std::cout << "Error: The target file failed to open" << std::endl;
		return 0;
	}

	return 1;
}