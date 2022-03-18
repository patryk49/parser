#include "tokenizer.hpp"


int main(){
	auto text_0 = sp::range("siusiacz");
	auto text_1 = sp::range("nabuchomonozor");
	auto text_2 = sp::range("dupsko");
	
	sp::push_range(names, text_0);
	sp::push_range(names, text_1);
	sp::push_range(names, text_2);



	auto tokens = make_tokens(
R"(
#import siusiacz

x = 2 + 3 * 5;
y = function(x) / (1 + sqrt(x));

while i:=0; i!=len(arr){
	defer i += 1;

	do_something();
}

return 1;

)");

	return 0;
}
