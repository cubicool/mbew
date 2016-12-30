#include "mbew.hpp"

#include <cstdio>

int main(int argc, char** argv) {
	if(argc < 2) {
		std::printf("Must specify WebM file.\n");

		return 1;
	}

	auto m = mbew::create(argv[1]);

	if(m->valid()) {
		while(m->iterate()) {
			auto index = m->iter.index();
			auto type = mbew::string(m->iter.type()).c_str();
			auto timestamp = (static_cast<double>(m->iter.timestamp() / 1000000)) / 1000.0;

			std::printf("index=%03u type=%s timestamp=%4.2fs\n", index, type, timestamp);
		}
	}

	else {
		std::printf("Error creating context (%s)\n", mbew::string(m->status()).c_str());

		return 1;
	}

	return 0;
}

