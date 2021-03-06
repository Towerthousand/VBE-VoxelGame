#include "Debugger.hpp"

int Debugger::numChunksDrawn = 0;
int Debugger::numChunksSkipped = 0;
int Debugger::numChunksDrawnShadow = 0;
int Debugger::numChunksSkippedShadow = 0;

Debugger::Debugger() {
}

Debugger::~Debugger() {
}

void Debugger::renderCustomInterface() const {
    ImGui::Begin("Controls", nullptr, ImVec2(0.18f*Window::getInstance()->getSize().x, 0.21f*Window::getInstance()->getSize().y), 0.9f, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowPos(ImVec2(0.78f*Window::getInstance()->getSize().x, 0.05f*Window::getInstance()->getSize().y), ImGuiCond_FirstUseEver);
    ImGui::Text("%s", std::string("ChunksDrawnDeferred: " + Utils::toString(numChunksDrawn)).c_str());
    ImGui::Text("%s", std::string("ChunksSkippedDeferred: " + Utils::toString(numChunksSkipped)).c_str());
    ImGui::Text("%s", std::string("ChunksDrawnShadow: " + Utils::toString(numChunksDrawnShadow)).c_str());
    ImGui::Text("%s", std::string("ChunksSkippedShadow: " + Utils::toString(numChunksSkippedShadow)).c_str());
    ImGui::Separator();
    ImGui::Columns(2, nullptr, true);
    ImGui::SetColumnOffset(0,0);
    ImGui::SetColumnOffset(1,0.7f*0.18f*Window::getInstance()->getSize().x);
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
