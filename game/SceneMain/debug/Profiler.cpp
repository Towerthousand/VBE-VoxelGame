#include "Profiler.hpp"
#include "SceneMain/DeferredContainer.hpp"
#include <cstring>
#include "SceneMain/Manager.hpp"

Profiler* Profiler::instance = nullptr;

Profiler::Profiler() {
	//setup singleton
	VBE_ASSERT(instance == nullptr, "Created two debug drawers");
	instance = this;

	//font texture
	tex = Texture2D::load(Storage::openAsset("debugFont.png"));
	tex.setFilter(GL_NEAREST, GL_NEAREST);

	setUpdatePriority(1000);
	setDrawPriority(1000);

	//setup UI model
	std::vector<Vertex::Attribute> elems = {
		Vertex::Attribute("a_position", Vertex::Attribute::Float, 2),
		Vertex::Attribute("a_texCoord", Vertex::Attribute::Float, 2),
		Vertex::Attribute("a_color", Vertex::Attribute::UnsignedByte, 4, Vertex::Attribute::ConvertToFloatNormalized)
	};

	Vertex::Format format(elems);
	model = Mesh(format, MeshSeparate::STREAM);

	//setup ui
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = Window::getInstance()->getSize().x;
	io.DisplaySize.y = Window::getInstance()->getSize().y;
	io.IniFilename = "imgui.ini";
	io.RenderDrawListsFn = &Profiler::renderHandle;
	io.SetClipboardTextFn = &Profiler::setClipHandle;
	io.GetClipboardTextFn = &Profiler::getClipHandle;
	io.IniSavingRate = -1.0f; //disable ini

	//add watcher
	Watcher* w = new Watcher();
	w->addTo(this);

	//set root node
	currentNode = &tree;
	pushMark("invalid", "invalid");
}

Profiler::~Profiler() {
	ImGui::Shutdown();
	instance = nullptr;
}

//static
void Profiler::pushMark(const std::string& name, const std::string& definition) {
	VBE_ASSERT(instance != nullptr, "Null debug drawer");
	VBE_ASSERT(instance->currentNode != nullptr, "Popped main node");
	for(Node& child : instance->currentNode->children) {
		if(child.name == name) {
			child.start();
			instance->currentNode = &child;
			return;
		}
	}
	instance->currentNode->children.push_back(Node(name,definition, instance->currentNode));
	instance->currentNode = &instance->currentNode->children.back();
}

//static
void Profiler::popMark() {
	VBE_ASSERT(instance != nullptr, "Null debug drawer");
	VBE_ASSERT(instance->currentNode != nullptr, "Too many popped nodes");
	instance->currentNode->stop();
	instance->currentNode = instance->currentNode->parent;
}

//static
void Profiler::setShown(bool shown) {
	VBE_ASSERT(instance != nullptr, "Null debug drawer");
	instance->showProfiler = shown;
}

//static
bool Profiler::isShown() {
	return (instance != nullptr && instance->showProfiler);
}

void Profiler::renderHandle(ImDrawList** const cmd_lists, int cmd_lists_count) {
	instance->render(cmd_lists, cmd_lists_count);
}

const char* Profiler::getClipHandle() {
	return instance->getClip();
}

void Profiler::setClipHandle(const char* text) {
	instance->setClipHandle(text);
}

void Profiler::render(ImDrawList** const cmd_lists, int cmd_lists_count) const {
	if (cmd_lists_count == 0)
		return;

	GL_ASSERT(glDisable(GL_CULL_FACE));
	GL_ASSERT(glDepthFunc(GL_ALWAYS));
	GL_ASSERT(glEnable(GL_SCISSOR_TEST));

	// Setup orthographic projection matrix
	const float width = ImGui::GetIO().DisplaySize.x;
	const float height = ImGui::GetIO().DisplaySize.y;
	mat4f perspective = glm::ortho(0.0f, width, height, 0.0f, -1.0f, +1.0f);
	Programs.get("debugDraw").uniform("MVP")->set(perspective);

	// Set texture for font
	Programs.get("debugDraw").uniform("fontTex")->set(&tex);

	// Render command lists
	for (int n = 0; n < cmd_lists_count; n++) {
		const ImDrawList* cmd_list = cmd_lists[n];
		model.setVertexData((const unsigned char*)cmd_list->vtx_buffer.begin(), cmd_list->vtx_buffer.size());

		int vtx_offset = 0;
		const ImDrawCmd* pcmd_end = cmd_list->commands.end();
		for (const ImDrawCmd* pcmd = cmd_list->commands.begin(); pcmd != pcmd_end; pcmd++) {
			GL_ASSERT(glScissor((int)pcmd->clip_rect.x, (int)(height - pcmd->clip_rect.w), (int)(pcmd->clip_rect.z - pcmd->clip_rect.x), (int)(pcmd->clip_rect.w - pcmd->clip_rect.y)));
			model.draw(Programs.get("debugDraw"), vtx_offset, pcmd->vtx_count);
			vtx_offset += pcmd->vtx_count;
		}
	}
	GL_ASSERT(glDisable(GL_SCISSOR_TEST));
	GL_ASSERT(glDepthFunc(GL_LEQUAL));
	GL_ASSERT(glEnable(GL_CULL_FACE));
}

const char* Profiler::getClip() const {
	//TODO
	return "";
}

void Profiler::setClip(const char* text) const {
	(void) text; //TODO
}

void Profiler::update(float deltaTime) {
	//end last frame
	popMark();//Update
	popMark();//Whole frame
	processNodeAverage(tree);
	if(timePassed >= sampleRate) {
		//update history
		timeAvgOffset = (timeAvgOffset + 1) % 100;
		for(auto it = hist.begin(); it != hist.end(); ++it) {
			it->second.past[timeAvgOffset] = (it->second.current/frameCount)*1000;
			it->second.current = 0.0f;
		}
		//update FPS
		timePassed -= sampleRate;
		FPS = float(frameCount)/sampleRate;
		frameCount = 0;
	}
	//do profiler
	if(Keyboard::justPressed(Keyboard::F1)) {
		showProfiler = !showProfiler;
		Mouse::setRelativeMode(!showProfiler);
	}
	setImguiIO(deltaTime);
	ImGui::NewFrame();
	if(showProfiler) {
		wsize = Window::getInstance()->getSize();
		ImGui::GetStyle().TreeNodeSpacing = 10;
		ImGui::GetStyle().WindowRounding = 6;
		ImGui::GetStyle().FrameRounding = 6;
		helpWindow();
		timeWindow();
		logWindow();
		//ImGui::ShowTestWindow();
	}
	//prepare for next frame
	frameCount++;
	timePassed += deltaTime;
	resetTree();
	pushMark("Draw", "Time spent issuing GL commands and drawing stuff on the screen");
}

void Profiler::draw() const {
	ImGui::Render();
	popMark(); //draw
	pushMark("Swap", "Time spent waiting for the GPU while commands/waits are executed.");
}

void Profiler::processNodeAverage(const Profiler::Node& n) {
	if(hist.find(n.name) == hist.end()) {
		hist.insert(std::pair<std::string, Historial>(n.name, Historial(hist.size())));
		memset(hist.at(n.name).past, 0, sizeof(float)*PROFILER_HIST_SIZE);
	}
	hist.at(n.name).current += n.getTime();
	for(const Node& child : n.children)
		processNodeAverage(child);
}

void Profiler::resetTree() {
	tree = Node("Whole Frame", "Total frame time. Lower bound of 16.66 ms if V-Sync is on.", nullptr);
	currentNode = &tree;
}

void Profiler::setImguiIO(float deltaTime) const {
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime == 0.0f ? 0.00001f : deltaTime;
	io.MouseWheel = Mouse::wheelMovement().y != 0 ? (Mouse::wheelMovement().y > 0 ? 1 : -1) : 0;
	io.KeyCtrl = Keyboard::pressed(Keyboard::LControl);
	io.KeyShift = Keyboard::pressed(Keyboard::LShift);
	io.MouseDown[0] = Mouse::pressed(Mouse::Left);
	io.MousePos = vec2f(Mouse::position());
	io.IniFilename = nullptr;
	io.DisplaySize = ImVec2(vec2f(Window::getInstance()->getSize()));
	io.UserData = nullptr;
	io.FontGlobalScale = 1.0f;
}

void Profiler::helpWindow() const {
	ImGui::Begin("Controls", nullptr, ImVec2(0.18f*wsize.x, 0.21f*wsize.y), windowAlpha, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::SetWindowPos(ImVec2(0.78f*wsize.x, 0.05f*wsize.y), ImGuiSetCondition_FirstUseEver);
	ImGui::Columns(2, nullptr, true);
	ImGui::SetColumnOffset(0,0);
	ImGui::SetColumnOffset(1,0.7f*0.18f*wsize.x);
	ImGui::Text("Move"); ImGui::NextColumn();
	ImGui::Text("W,A,S,D"); ImGui::NextColumn();
	ImGui::Text("Look Around"); ImGui::NextColumn();
	ImGui::Text("Mouse"); ImGui::NextColumn();
	ImGui::Text("Jump"); ImGui::NextColumn();
	ImGui::Text("Space"); ImGui::NextColumn();
	ImGui::Text("Place a light"); ImGui::NextColumn();
	ImGui::Text("L"); ImGui::NextColumn();
	ImGui::Text("Toggle interface"); ImGui::NextColumn();
	ImGui::Text("F1"); ImGui::NextColumn();
	ImGui::Text("Toggle sun camera"); ImGui::NextColumn();
	ImGui::Text("Q"); ImGui::NextColumn();
	ImGui::Text("Decrease sun angle"); ImGui::NextColumn();
	ImGui::Text("Z"); ImGui::NextColumn();
	ImGui::Text("Increase sun angle"); ImGui::NextColumn();
	ImGui::Text("X"); ImGui::NextColumn();
	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::Text("While showing the interface,\nthe mouse will be visible.\nHover the profiler graphs for\nmore info");
	ImGui::End();
}

void Profiler::timeWindow() const {
	ImGui::Begin("Frame Times", nullptr, ImVec2(0.23f*wsize.x,0.55f*wsize.y), windowAlpha);
	ImGui::SetWindowPos(ImVec2(0.025f*wsize.x, 0.05f*wsize.y), ImGuiSetCondition_FirstUseEver);
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);
	ImGui::Text("With V-Sync enabled, frame time will\nnot go below 16ms");
	ImGui::Text("FPS: %i", FPS);
	ImGui::Separator();
	uiProcessNode(tree);
	ImGui::End();
}

void Profiler::logWindow() const {
	std::string log = Log::getContents();
	ImGui::Begin("Log", nullptr, ImVec2(0.34f*wsize.x, 0.31f*wsize.y), windowAlpha);
	ImGui::SetWindowPos(ImVec2(0.025f*wsize.x, 0.61f*wsize.y), ImGuiSetCondition_FirstUseEver);
	ImGui::BeginChild("Log");
	ImGui::TextUnformatted(&(*log.begin()), &(*log.end()));
	ImGui::EndChild();
	ImGui::End();
}

void Profiler::uiProcessNode(const Profiler::Node& n) const {
	const Historial& nHist = hist.at(n.name);
	std::string currTime = Utils::toString(nHist.past[timeAvgOffset],4,2,true);
	std::string tag = std::string(n.name + " Time (curr: ") + currTime + " ms)";
	if (ImGui::TreeNode((void*)nHist.id, tag.c_str())) {
		ImGui::PlotLines("25ms\n\n\n\n0 ms", nHist.past, 100, timeAvgOffset, currTime.c_str(), 0.00f, 25.0f, vec2f(350,60));
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("");
			ImGui::BeginTooltip();
			ImGui::Text(n.desc.c_str());
			ImGui::EndTooltip();
		}
		for(const Node& child : n.children)
			uiProcessNode(child);
		ImGui::TreePop();
	}
}

Profiler::Watcher::Watcher() {
	setUpdatePriority(-1000);
	setDrawPriority(-1000);
}

Profiler::Watcher::~Watcher() {
}

void Profiler::Watcher::update(float deltaTime) {
	(void) deltaTime;
	popMark(); //swap
	pushMark("Update", "Time spent updating game logic");
}

void Profiler::Watcher::draw() const {
}
