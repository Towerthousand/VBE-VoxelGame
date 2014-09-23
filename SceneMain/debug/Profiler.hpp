#ifndef PROFILER_HPP
#define PROFILER_HPP
#include "imgui.hpp"

class DeferredContainer;
class Profiler : public GameObject {
	public:
		Profiler();
		~Profiler();

		static void renderHandle(ImDrawList** const cmd_lists, int cmd_lists_count);
		static const char*getClipHandle();
		static void setClipHandle(const char* text, const char* text_end);

	private:
		static Profiler* instance;

		void render(ImDrawList** const cmd_lists, int cmd_lists_count) const;
		const char* getClip() const;
		void setClip(const char* text, const char* text_end) const;
		void update(float deltaTime);
		void draw() const;

		Model model;
		Texture2D* tex;
		bool show_test;
		bool show_other;
		float floatSlider;
};

#endif // PROFILER_HPP
