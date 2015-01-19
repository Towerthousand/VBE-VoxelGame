#ifndef PROFILER_HPP
#define PROFILER_HPP
#include <VBE-Profiler/profiler/imgui.hpp>
#include <VBE/VBE.hpp>
#include <VBE-Scenegraph/VBE-Scenegraph.hpp>

#define PROFILER_HIST_SIZE 50

class DeferredContainer;
class Profiler : public GameObject {
	public:
		Profiler();
		Profiler(std::string vertShader, std::string fragShader);
		~Profiler();

		static void pushMark(const std::string& name, const std::string& definition);
		static void popMark();
		static bool isShown();
		static void setShown(bool shown);
		static bool isLogShown();
		static void setShowLog(bool shown);
		static bool isTimeShown();
		static void setShowTime(bool shown);

	protected:
		virtual void render(ImDrawList** const cmd_lists, int cmd_lists_count) const;
		virtual void renderCustomInterface() const;

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

		class Watcher final : public GameObject {
			public:
				Watcher();
				~Watcher();

				void update(float deltaTime);
				void draw() const;
		};

		struct Historial final {
				Historial(unsigned long int id) : id(id) {}
				const unsigned long int id = 0;
				float current = 0.0f;
				float past[PROFILER_HIST_SIZE];
		};

		static void renderHandle(ImDrawList** const cmd_lists, int cmd_lists_count);
		static const char*getClipHandle();
		static void setClipHandle(const char* text);

		//callback
		const char* getClip() const;
		//callback
		void setClip(const char* text) const;

		void update(float deltaTime) final;
		void draw() const final;
		void processNodeAverage(const Node& n);
		void resetTree();
		void setImguiIO(float deltaTime) const;
		void timeWindow() const;
		void logWindow() const;
		void uiProcessNode(const Node& n) const;

		static Profiler* instance;
		static std::string defaultVS;
		static std::string defaultFS;

		int timeAvgOffset = -1;
		mutable int frameCount = 0;
		mutable float timePassed = 0.0f;
		mutable int FPS = 0;
		bool showProfiler = false;
		bool showTime = true;
		bool showLog = true;
		float sampleRate = 0.5f;
		float windowAlpha = 0.9f;
		vec2ui wsize = vec2ui(0,0);
		Node tree;
		Node* currentNode = nullptr;
		std::map<std::string, Historial> hist;
		mutable Mesh model;
		Texture2D tex;
		ShaderProgram program;
		mutable std::string clip = "";
};

#endif // PROFILER_HPP
