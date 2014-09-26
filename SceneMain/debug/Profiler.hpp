#ifndef PROFILER_HPP
#define PROFILER_HPP
#include "imgui.hpp"

class DeferredContainer;
class Profiler : public GameObject {
	public:
		Profiler();
		~Profiler();

		enum IntVar {
			PlayerChunksDrawn = 0,
			SunChunksDrawn,
			ColumnsAdded,
			INT_VAR_COUNT
		};

		enum TimeVar {
			PlayerChunkRebuildTime = 0,
			PlayerChunkDrawTime,
			PlayerChunkBFSTime,
			SunChunkRebuildTime,
			SunChunkDrawTime,
			SunChunkBFSTime,
			WorldUpdateTime,
			FrameTime,
			UpdateTime,
			DrawTime,
			TIME_VAR_COUNT
		};

		static int intVars[INT_VAR_COUNT];
		static float timeVars[TIME_VAR_COUNT];
		//static float floatVars[TIME_VAR_COUNT];

		//Public callbacks
		static void renderHandle(ImDrawList** const cmd_lists, int cmd_lists_count);
		static const char*getClipHandle();
		static void setClipHandle(const char* text, const char* text_end);

	private:
		float timeAccumVars[TIME_VAR_COUNT];
		float timeAvgVars[TIME_VAR_COUNT];
		//internal debug vars
		mutable float updateTimeStart;
		mutable float updateTimeEnd;
		mutable float drawTimeStart;
		mutable float drawTimeEnd;

		mutable int frameCount;
		mutable float timePassed;
		mutable int FPS;
		bool showProfiler;
		float sampleRate;

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
