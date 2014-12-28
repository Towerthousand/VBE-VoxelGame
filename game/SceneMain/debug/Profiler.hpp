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

		enum Vec3fVar {
			PlayerPos = 0,
			VEC3F_VAR_COUNT
		};

		enum TimeVar {
			PlayerChunkRebuildTime = 0,
			PlayerChunkDrawTime,
			PlayerChunkBFSTime,
			ShadowChunkRebuildTime,
			ShadowChunkDrawTime,
			ShadowChunkBFSTime,
			WorldUpdateTime,
			BlurPassTime,
			ForwardPassTime,
			DeferredPassTime,
			ShadowBuildPassTime,
			LightPassTime,
			AmbinentShadowPassTime,
			ShadowWorldTime,
			PlayerWorldTime,
			FrameTime,
			UpdateTime,
			UIDrawTime,
			DrawTime,
			SwapTime,
			TIME_VAR_COUNT
		};

		static int intVars[INT_VAR_COUNT];
		static float timeVars[TIME_VAR_COUNT];
		static vec3f vec3fVars[VEC3F_VAR_COUNT];

		static bool shown();

		//Public callbacks
		static void renderHandle(ImDrawList** const cmd_lists, int cmd_lists_count);
		static const char*getClipHandle();
		static void setClipHandle(const char* text, const char* text_end);

	private:
		class Watcher : public GameObject {
			public:
				Watcher();
				~Watcher();

				void update(float deltaTime);
				void draw() const;
		};

		float timeAccumVars[TIME_VAR_COUNT];
		float timeAvgVars[TIME_VAR_COUNT][100];
		int timeAvgOffset;

		//internal debug vars
		mutable float updateTimeStart;
		mutable float updateTimeEnd;
		mutable float drawTimeStart;
		mutable float drawTimeEnd;
		mutable float swapTimeStart;
		mutable float swapTimeEnd;

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

		mutable Mesh model;
		Texture2D tex;
		mutable std::string clip;
};

#endif // PROFILER_HPP
