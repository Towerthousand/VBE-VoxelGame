#ifndef PROFILER_HPP
#define PROFILER_HPP
#include "imgui.hpp"

class DeferredContainer;
class Profiler : public GameObject {
	public:
		Profiler();
		~Profiler();

		//Debug variables
		static int playerChunksDrawn;
		static int sunChunksDrawn;
		static int columnsGenerated;
		static float playerChunkRebuildTime;
		static float playerChunkDrawTime;
		static float playerChunkBFSTime;
		static float sunChunkRebuildTime;
		static float sunChunkDrawTime;
		static float sunChunkBFSTime;
		static float worldUpdateTime;

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
		mutable float frameTimeAccum;
		mutable float updateTimeAccum;
		mutable float drawTimeAccum;
		mutable int frameCount;
		mutable float timePassedAccum;
		mutable float playerRebuildTimeAccum;
		mutable float playerDrawTimeAccum;
		mutable float playerBFSTimeAccum;
		mutable float sunDrawTimeAccum;
		mutable float sunRebuildTimeAccum;
		mutable float sunBFSTimeAccum;
		mutable float worldUpdateTimeAccum;
		mutable float avgFrameTime;
		mutable float avgUpdateTime;
		mutable float avgDrawTime;
		mutable float avgPlayerRebuildTime;
		mutable float avgPlayerDrawTime;
		mutable float avgPlayerBFSTime;
		mutable float avgSunRebuildTime;
		mutable float avgSunBFSTime;
		mutable float avgSunDrawTime;
		mutable float avgWorldUpdateTime;
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
