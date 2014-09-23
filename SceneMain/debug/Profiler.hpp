#ifndef PROFILER_HPP
#define PROFILER_HPP
#include "imgui.hpp"

class DeferredContainer;
class Profiler : public GameObject {
	public:
		Profiler();
		~Profiler();

		//Debug variables
		static int cameraChunksDrawn;
		static int sunChunksDrawn;
		static int columnsGenerated;

		//Public callbacks
		static void renderHandle(ImDrawList** const cmd_lists, int cmd_lists_count);
		static const char*getClipHandle();
		static void setClipHandle(const char* text, const char* text_end);

	private:
		//internal debug vars
		mutable float frameTimeStart;
		mutable float frameTimeEnd;
		mutable float updateTimeStart;
		mutable float updateTimeEnd;
		mutable float drawTimeStart;
		mutable float drawTimeEnd;
		mutable int frameCount;
		mutable float timePassed;
		mutable int FPS;
		mutable int colsPerSecond;

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
