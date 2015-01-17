#ifndef PROFILER_HPP
#define PROFILER_HPP
#include "imgui.hpp"
#define PROFILER_HIST_SIZE 100

class DeferredContainer;
class Profiler : public GameObject {
	public:
		Profiler();
		~Profiler();

		static void pushMark(const std::string& name, const std::string& definition);
		static void popMark();
		static bool isShown();
		static void setShown(bool shown);

		//public callbacks (must be static :()
		static void renderHandle(ImDrawList** const cmd_lists, int cmd_lists_count);
		static const char*getClipHandle();
		static void setClipHandle(const char* text);

	private:
		struct Node {
				Node() {start();}
				Node(const std::string& name, const std::string& desc, Node* parent)
					: parent(parent), name(name), desc(desc) {start();}
				~Node() {}

				float getTime() const {return time;}
				void start() {time0 = Clock::getSeconds();}
				void stop() {time1 = Clock::getSeconds(); time += time1-time0; time0 = 0.0f;}

				Node* parent = nullptr;
				std::list<Node> children;
				std::string name = std::string("invalid");
				std::string desc = std::string("invalid");
			private:
				float time = 0.0f;
				float time0 = 0.0f;
				float time1 = 0.0f;
		};

		class Watcher : public GameObject {
			public:
				Watcher();
				~Watcher();

				void update(float deltaTime);
				void draw() const;
		};

		struct Historial {
				Historial(unsigned long int id) : id(id) {}
				const unsigned long int id = 0;
				float current = 0.0f;
				float past[PROFILER_HIST_SIZE];
		};

		//instanced callbacks
		void render(ImDrawList** const cmd_lists, int cmd_lists_count) const;
		const char* getClip() const;
		void setClip(const char* text) const;
		void update(float deltaTime);
		void draw() const;
		void processNodeAverage(const Node& n);
		void resetTree();
		void setImguiIO(float deltaTime) const;
		void helpWindow() const;
		void timeWindow() const;
		void logWindow() const;
		void uiProcessNode(const Node& n) const;

		int timeAvgOffset = -1;
		mutable int frameCount = 0;
		mutable float timePassed = 0.0f;
		mutable int FPS = 0;
		bool showProfiler = false;
		float sampleRate = 0.5f;
		float windowAlpha = 0.9f;
		Node tree;
		Node* currentNode = nullptr;
		std::map<std::string, Historial> hist;
		vec2ui wsize = vec2ui(0,0);

		//System stuff
		static Profiler* instance;

		mutable Mesh model;
		Texture2D tex;
		mutable std::string clip = "";
};

#endif // PROFILER_HPP
