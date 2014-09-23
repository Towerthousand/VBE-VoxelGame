#ifndef PROFILER_HPP
#define PROFILER_HPP
#include "imgui.hpp"

class DeferredContainer;
class Profiler : public GameObject {
	public:
		Profiler();
		~Profiler();

		//Debug variables
		static float frameTimeStart;
		static float frameTimeEnd;
		static float updateTimeStart;
		static float updateTimeEnd;
		static float drawTimeStart;
		static float drawTimeEnd;
		static int cameraChunksDrawn;
		static int sunChunksDrawn;
		static int columnsGenerated;

		//Public callbacks
		static void renderHandle(ImDrawList** const cmd_lists, int cmd_lists_count);
		static const char*getClipHandle();
		static void setClipHandle(const char* text, const char* text_end);

	private:
		//internal debug vars
		int frameCount;
		float timePassed;
		int FPS;
		int colsPerSecond;

		//System stuff
		static Profiler* instance;

		void render(ImDrawList** const cmd_lists, int cmd_lists_count) const;
		const char* getClip() const;
		void setClip(const char* text, const char* text_end) const;
		void update(float deltaTime);
		void draw() const;

		Model model;
		Texture2D* tex;
		mutable std::string clip;
};

#endif // PROFILER_HPP
