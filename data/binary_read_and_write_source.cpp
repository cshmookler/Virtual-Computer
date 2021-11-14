#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{
	if(argc != 4)
	{
		std::cout << "Error: Incorrect number of arguments" << std::endl;
		return 0;
	}

	if((char)argv[1][0] == (char)"r"[0])
	{
		char c;
		int tempNum,
			num;
		bool byteType = true;

		std::ifstream source(argv[2], std::ios::binary);
		if(!source.is_open())
		{
			std::cout << "Error: The source file failed to open" << std::endl;
			return 0;
		}

		std::ofstream target(argv[3], std::ios::trunc);
		if(!target.is_open())
		{
			std::cout << "Error: The target file failed to open" << std::endl;
			return 0;
		}

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

		source.close();
		target.close();
	}
	else if((char)argv[1][0] == (char)"w"[0])
	{
		int tempNum,
			numHigh,
			numLow;

		std::ifstream source(argv[2]);
		if(!source.is_open())
		{
			std::cout << "Error: The source file failed to open" << std::endl;
			return 0;
		}

		std::ofstream target(argv[3], std::ios::binary | std::ios::trunc);
		if(!target.is_open())
		{
			std::cout << "Error: The target file failed to open" << std::endl;
			return 0;
		}

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

		source.close();
		target.close();
	}
	else
	{
		std::cout << "Error: Unknown operation '" << argv[1] << "'" << std::endl;
		return 0;
	}

	std::cout << "\n    Done!" << std::endl;
	return 1;
}