#ifndef DEBUGDRAWER_HPP
#define DEBUGDRAWER_HPP
#include "imgui.hpp"

class DeferredContainer;
class DebugDrawer : public GameObject {
	public:
		DebugDrawer();
		~DebugDrawer();

		static void renderHandle(ImDrawList** const cmd_lists, int cmd_lists_count);
		static const char*getClipHandle();
		static void setClipHandle(const char* text, const char* text_end);

	private:
		static DebugDrawer* instance;

		void render(ImDrawList** const cmd_lists, int cmd_lists_count) const;
		const char* getClip() const;
		void setClip(const char* text, const char* text_end) const;
		void update(float deltaTime);
		void draw() const;

		DeferredContainer* renderer;
		Model model;
		Texture2D* tex;
};

#endif // DEBUGDRAWER_HPP
