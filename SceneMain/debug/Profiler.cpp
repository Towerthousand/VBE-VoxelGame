#include "Profiler.hpp"
#include "SceneMain/DeferredContainer.hpp"

int Profiler::playerChunksDrawn = 0;
int Profiler::sunChunksDrawn = 0;
int Profiler::columnsGenerated = 0;
float Profiler::playerChunkRebuildTime = 0.0f;
float Profiler::playerChunkDrawTime = 0.0f;
float Profiler::playerChunkBFSTime = 0.0f;
float Profiler::sunChunkRebuildTime = 0.0f;
float Profiler::sunChunkDrawTime = 0.0f;
float Profiler::sunChunkBFSTime = 0.0f;
float Profiler::worldUpdateTime = 0.0f;
Profiler* Profiler::instance = nullptr;

Profiler::Profiler() :
	frameTimeStart(0.0f), frameTimeEnd(0.0f),
	updateTimeStart(0.0f), updateTimeEnd(0.0f),
	drawTimeStart(0.0f), drawTimeEnd(0.0f),
	frameTimeAccum(0.0f), updateTimeAccum(0.0f), drawTimeAccum(0.0f),
	frameCount(0), timePassedAccum(0.0f),
	playerRebuildTimeAccum(0.0f), playerDrawTimeAccum(0.0f), playerBFSTimeAccum(0.0f),
	sunDrawTimeAccum(0.0f), sunRebuildTimeAccum(0.0f), sunBFSTimeAccum(0.0f),
	avgFrameTime(0.0f), avgUpdateTime(0.0f), avgDrawTime(0.0f),
	avgPlayerRebuildTime(0.0f), avgPlayerDrawTime(0.0f), avgPlayerBFSTime(0.0f),
	avgSunRebuildTime(0.0f), avgSunBFSTime(0.0f), avgSunDrawTime(0.0f),
	worldUpdateTimeAccum(0.0f),
	FPS(0), showProfiler(true), sampleRate(0.5f),
	tex(nullptr) {
	VBE_ASSERT(instance == nullptr, "Created two debug drawers");
	instance = this;

	tex = Texture2D::createFromFile("data/debugFont.png");
	tex->setFilter(GL_NEAREST, GL_NEAREST);

	setUpdatePriority(100);
	setDrawPriority(100);

	model.program = Programs.get("debugDraw");
	std::vector<Vertex::Element> elems = {
		Vertex::Element(Vertex::Attribute::Position, Vertex::Element::Float, 2),
		Vertex::Element(Vertex::Attribute::TexCoord, Vertex::Element::Float, 2),
		Vertex::Element(Vertex::Attribute::Color, Vertex::Element::UnsignedByte, 4, Vertex::Element::ConvertToFloatNormalized)
	};

	Vertex::Format format(elems);
	model.mesh = Mesh::loadEmpty(format, Mesh::STREAM, false);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = Environment::getScreen()->getWidth();
	io.DisplaySize.y = Environment::getScreen()->getHeight();
	io.IniFilename = "imgui.ini";
	io.RenderDrawListsFn = &Profiler::renderHandle;
	io.SetClipboardTextFn = &Profiler::setClipHandle;
	io.GetClipboardTextFn = &Profiler::getClipHandle;
}

Profiler::~Profiler() {
	delete model.mesh;
	delete tex;
	ImGui::Shutdown();
	instance = nullptr;
}

void Profiler::renderHandle(ImDrawList** const cmd_lists, int cmd_lists_count) {
	instance->render(cmd_lists, cmd_lists_count);
}

const char* Profiler::getClipHandle() {
	return instance->getClip();
}

void Profiler::setClipHandle(const char* text, const char* text_end) {
	instance->setClipHandle(text, text_end);
}

void Profiler::render(ImDrawList** const cmd_lists, int cmd_lists_count) const {
	//if(renderer->getMode() != DeferredContainer::Forward) return;
	if (cmd_lists_count == 0)
		return;

	GL_ASSERT(glDisable(GL_CULL_FACE));
	GL_ASSERT(glDepthFunc(GL_ALWAYS));
	GL_ASSERT(glEnable(GL_SCISSOR_TEST));

	// Setup orthographic projection matrix
	const float width = ImGui::GetIO().DisplaySize.x;
	const float height = ImGui::GetIO().DisplaySize.y;
	mat4f perspective = glm::ortho(0.0f, width, height, 0.0f, -1.0f, +1.0f);
	model.program->uniform("MVP")->set(perspective);

	// Set texture for font
	model.program->uniform("fontTex")->set(tex);

	// Render command lists
	for (int n = 0; n < cmd_lists_count; n++) {
		const ImDrawList* cmd_list = cmd_lists[n];
		model.mesh->setVertexData((const unsigned char*)cmd_list->vtx_buffer.begin(), cmd_list->vtx_buffer.size());

		int vtx_offset = 0;
		const ImDrawCmd* pcmd_end = cmd_list->commands.end();
		for (const ImDrawCmd* pcmd = cmd_list->commands.begin(); pcmd != pcmd_end; pcmd++) {
			GL_ASSERT(glScissor((int)pcmd->clip_rect.x, (int)(height - pcmd->clip_rect.w), (int)(pcmd->clip_rect.z - pcmd->clip_rect.x), (int)(pcmd->clip_rect.w - pcmd->clip_rect.y)));
			model.draw(vtx_offset, pcmd->vtx_count);
			vtx_offset += pcmd->vtx_count;
		}
	}
	GL_ASSERT(glDisable(GL_SCISSOR_TEST));
	GL_ASSERT(glDepthFunc(GL_LEQUAL));
	GL_ASSERT(glEnable(GL_CULL_FACE));
}

const char* Profiler::getClip() const {
	char* c = SDL_GetClipboardText();
	clip = std::string(c);
	SDL_free(c);
	return clip.c_str();
}

void Profiler::setClip(const char* text, const char* text_end) const {
	if (!text_end)
		text_end = text + strlen(text);

	if (*text_end == 0) {
		// Already got a zero-terminator at 'text_end', we don't need to add one
		SDL_SetClipboardText(text);
	}
	else {
		// Add a zero-terminator because sdl function doesn't take a size
		char* buf = (char*)malloc(text_end - text + 1);
		memcpy(buf, text, text_end-text);
		buf[text_end-text] = '\0';
		SDL_SetClipboardText(buf);
		free(buf);
	}
}

void Profiler::update(float deltaTime) {
	if(Environment::getKeyboard()->isKeyPressed(Keyboard::F1)) showProfiler = !showProfiler;
	updateTimeEnd = Environment::getClock();
	frameTimeEnd = updateTimeEnd;
	//INPUT
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime == 0.0f ? 0.00001f : deltaTime;
	io.MouseWheel = Environment::getMouse()->getMouseWheelMovement().y != 0 ? (Environment::getMouse()->getMouseWheelMovement().y > 0 ? 1 : -1) : 0;
	io.KeyCtrl = Environment::getKeyboard()->isKeyHeld(Keyboard::LControl);
	io.KeyShift = Environment::getKeyboard()->isKeyHeld(Keyboard::LShift);
	io.MouseDown[0] = Environment::getMouse()->isButtonHeld(Mouse::Left);
	io.MousePos = vec2f(Environment::getMouse()->getMousePos());
	ImGui::NewFrame();
	//UPDATE INTERNAL DEBUG VARS
	timePassedAccum += deltaTime;
	frameTimeAccum += frameTimeEnd-frameTimeStart;
	updateTimeAccum += updateTimeEnd-updateTimeStart;
	drawTimeAccum += drawTimeEnd-drawTimeStart;
	playerRebuildTimeAccum += playerChunkRebuildTime;
	playerDrawTimeAccum += playerChunkDrawTime;
	playerBFSTimeAccum += playerChunkBFSTime;
	sunDrawTimeAccum += sunChunkDrawTime;
	sunRebuildTimeAccum += sunChunkRebuildTime;
	sunBFSTimeAccum += sunChunkBFSTime;
	worldUpdateTimeAccum += worldUpdateTime;
	if(timePassedAccum >= sampleRate) {
		timePassedAccum -= sampleRate;
		FPS = float(frameCount)/sampleRate;
		avgFrameTime = frameTimeAccum/float(frameCount);
		avgUpdateTime = updateTimeAccum/float(frameCount);
		avgDrawTime = drawTimeAccum/float(frameCount);
		avgPlayerRebuildTime = playerRebuildTimeAccum/float(frameCount);
		avgPlayerDrawTime =  playerDrawTimeAccum/float(frameCount);
		avgPlayerBFSTime =  playerBFSTimeAccum/float(frameCount);
		avgSunRebuildTime = sunRebuildTimeAccum/float(frameCount);
		avgSunDrawTime = sunDrawTimeAccum/float(frameCount);
		avgSunBFSTime = sunBFSTimeAccum/float(frameCount);
		avgWorldUpdateTime = worldUpdateTime/float(frameCount);
		frameTimeAccum = 0.0f;
		updateTimeAccum = 0.0f;
		drawTimeAccum = 0.0f;
		playerRebuildTimeAccum = 0.0f;
		playerDrawTimeAccum = 0.0f;
		playerBFSTimeAccum = 0.0f;
		sunDrawTimeAccum = 0.0f;
		sunRebuildTimeAccum = 0.0f;
		sunBFSTimeAccum = 0.0f;
		frameCount = 0;
	}
	frameCount++;
	//UI
	if(showProfiler) {
		ImGui::SetNewWindowDefaultPos(ImVec2(50, 20));
		ImGui::Begin("VoxelGame Profiler", nullptr, ImVec2(350,310));
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);
		ImGui::Text("With V-Sync enabled, frame time will\nnot go below 0.16");
		ImGui::Text("FPS: %i", FPS);
		ImGui::Text("Frame time: %f", avgFrameTime);
		ImGui::Separator();
		ImGui::Text("Draw time: %f", avgDrawTime);
		ImGui::Text("Player Chunk Rebuilding time: %f", avgPlayerRebuildTime);
		ImGui::Text("Player Chunk Drawing time: %f", avgPlayerDrawTime);
		ImGui::Text("Player Camera Chunks: %i", playerChunksDrawn);
		ImGui::Text("Player Chunk BFS time: %f", avgPlayerBFSTime);
		ImGui::Text("Sun Chunk Rebuilding time: %f", avgSunRebuildTime);
		ImGui::Text("Sun Chunk Drawing time: %f", avgSunDrawTime);
		ImGui::Text("Sun Camera Chunks: %i", sunChunksDrawn);
		ImGui::Text("Sun Chunk BFS time: %f", avgSunBFSTime);
		ImGui::Separator();
		ImGui::Text("Update time: %f", avgUpdateTime);
		ImGui::Text("World Update time: %f", avgWorldUpdateTime);
		ImGui::End();
	}
	//ImGui::ShowTestWindow();
	//RESET VARS FOR NEXT FRAME
	playerChunksDrawn = 0;
	sunChunksDrawn = 0;
	columnsGenerated = 0;
	playerChunkRebuildTime = 0.0f;
	playerChunkDrawTime = 0.0f;
	playerChunkBFSTime = 0.0f;
	sunChunkRebuildTime = 0.0f;
	sunChunkDrawTime = 0.0f;
	sunChunkBFSTime = 0.0f;
	drawTimeStart = updateTimeEnd;
	frameTimeStart = drawTimeStart;
}

void Profiler::draw() const {
	ImGui::Render();
	drawTimeEnd = Environment::getClock();
	updateTimeStart = drawTimeEnd;
}
