#include "finders_interface.h"

using namespace rectpack2D;
void __cdecl std::_Xlength_error(char const *)
{
    std::terminate();
}
extern "C"
{
typedef callback_result (*successful_insertion_callback)(rect_xywh rect, void *user_data);
typedef callback_result (*failed_insertion_callback)(rect_xywh rect, void *user_data);

struct Rectpack_Data
{
    int max_side;
    int discard_step;
    void *rectangles;
    successful_insertion_callback successful_insertion_proc;
    failed_insertion_callback failed_insertion_proc;
    void *user_data;
};

__declspec(dllexport) 
Rectpack_Data *init_rectpack(int max_side, int discard_step, 
                             successful_insertion_callback successful_insertion_proc,
                             failed_insertion_callback failed_insertion_proc, void *user_data)
{
    Rectpack_Data *result = new Rectpack_Data();
    result->max_side = max_side;
    result->discard_step = discard_step;
    result->rectangles = new std::vector<rect_xywh>();
    result-> successful_insertion_proc = successful_insertion_proc;
    result->failed_insertion_proc = failed_insertion_proc;
    result->user_data = user_data;
    return result;
}

__declspec(dllexport) 
void add_rectangle(Rectpack_Data *data, int width, int height)
{
    std::vector<rect_xywh> *rectangles = (std::vector<rect_xywh>*)data->rectangles;
    rectangles->emplace_back(rect_xywh(-1, -1, width, height));
}

struct Rectpack_Result
{
    rect_wh result_size;
    rect_xywh *data;
    int count;
};

__declspec(dllexport) 
Rectpack_Result pack_rectangles(Rectpack_Data *data)
{
    using spaces_type = empty_spaces<false, default_empty_spaces>;
    std::vector<rect_xywh> *rectangles = (std::vector<rect_xywh>*)data->rectangles;

    const auto result_size = find_best_packing<spaces_type>(
        *rectangles,
        make_finder_input(data->max_side, data->discard_step, data->successful_insertion_proc,
                          data->failed_insertion_proc, flipping_option::DISABLED, data->user_data)
    );

    Rectpack_Result result;
    result.data = rectangles->data();
    result.count = (int)rectangles->size();
    result.result_size = result_size;
    return result;
}

__declspec(dllexport) 
void deinit_rectpack(Rectpack_Data *data)
{
    std::vector<rect_xywh> *rectangles = (std::vector<rect_xywh>*)data->rectangles;
    delete rectangles;
    delete data;
}
}


// #define DEMO
#ifdef DEMO
#include <vector>
#include <iostream>


int main()
{
    flipping_option runtime_flipping_mode = flipping_option::DISABLED;

    using spaces_type = empty_spaces<false, default_empty_spaces>;
    using rect_type = output_rect_t<spaces_type>;

    auto report_successful = [](rect_type&) {
		return callback_result::CONTINUE_PACKING;
	};

	auto report_unsuccessful = [](rect_type&) {
		return callback_result::ABORT_PACKING;
	};

    const auto max_side = 4096;
    const auto discard_step = -4;
    std::vector<rect_type> rectangles;

	rectangles.emplace_back(rect_xywh(0, 0, 20, 40));
	rectangles.emplace_back(rect_xywh(0, 0, 120, 40));
	rectangles.emplace_back(rect_xywh(0, 0, 85, 59));
	rectangles.emplace_back(rect_xywh(0, 0, 199, 380));
	rectangles.emplace_back(rect_xywh(0, 0, 85, 875));

    auto report_result = [&rectangles](const rect_wh& result_size) {
		std::cout << "Resultant bin: " << result_size.w << " " << result_size.h << std::endl;

		for (const auto& r : rectangles) {
			std::cout << r.x << " " << r.y << " " << r.w << " " << r.h << std::endl;
		}
	};

    const auto result_size = find_best_packing<spaces_type>(
			rectangles,
			make_finder_input(
				max_side,
				discard_step,
				report_successful,
				report_unsuccessful,
				runtime_flipping_mode
			)
		);

		report_result(result_size);
}
#endif