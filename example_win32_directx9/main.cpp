// Dear ImGui: standalone example application for DirectX 9

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <map>
#include <memory>

// Data
static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static bool                     g_DeviceLost = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ImGui Builder Classes
enum class ElementType {
    CHECKBOX,
    BUTTON,
    SLIDER_FLOAT,
    SLIDER_INT,
    INPUT_TEXT,
    INPUT_INT,
    INPUT_FLOAT,
    COMBO,
    LISTBOX,
    COLOR_PICKER,
    SEPARATOR,
    TEXT,
    BULLET_TEXT,
    TREE_NODE,
    COLLAPSING_HEADER,
    TAB_BAR,
    TAB_ITEM,
    MENU_BAR,
    MENU_ITEM,
    POPUP,
    TOOLTIP,
    PROGRESS_BAR,
    IMAGE_BUTTON,
    RADIO_BUTTON,
    SELECTABLE,
    SPACING,
    SAME_LINE,
    NEW_LINE,
    INDENT,
    UNINDENT,
    GROUP,
    CHILD_WINDOW,
    COLUMNS,
    TABLE,
    PLOT_LINES,
    PLOT_HISTOGRAM
};

struct ImGuiElement {
    ElementType type;
    std::string label;
    std::string id;
    bool enabled = true;
    bool visible = true;

    // Values for different element types
    bool bool_value = false;
    int int_value = 0;
    float float_value = 0.0f;
    std::string text_value = "";
    ImVec4 color_value = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    std::vector<std::string> combo_items;
    int selected_item = 0;
    float min_value = 0.0f;
    float max_value = 100.0f;

    // Style properties
    ImVec2 size = ImVec2(0, 0);
    ImVec4 text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 bg_color = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);

    // Tree structure
    std::vector<std::shared_ptr<ImGuiElement>> children;
    bool is_open = false;

    ImGuiElement(ElementType t, const std::string& l) : type(t), label(l), id(l + "##" + std::to_string(rand())) {}
};

class ImGuiBuilder {
private:
    std::vector<std::shared_ptr<ImGuiElement>> elements;
    std::shared_ptr<ImGuiElement> selected_element = nullptr;
    bool show_menu = false;
    bool show_properties = true;
    bool show_element_tree = true;
    bool show_preview = true;

    // Builder state
    char new_element_name[256] = "New Element";
    ElementType selected_type = ElementType::BUTTON;

    // Menu creation
    bool create_menu = false;
    char menu_name[256] = "My Menu";

public:
    void Render() {
        // Main menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Menu")) {
                    create_menu = true;
                }
                if (ImGui::MenuItem("Save Menu")) {
                    // Save functionality
                }
                if (ImGui::MenuItem("Load Menu")) {
                    // Load functionality
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Properties", nullptr, &show_properties);
                ImGui::MenuItem("Element Tree", nullptr, &show_element_tree);
                ImGui::MenuItem("Preview", nullptr, &show_preview);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    // About dialog
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        // Create Menu Checkbox
        ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_FirstUseEver);
        ImGui::Begin("Menu Creator", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Checkbox("Make Menu", &create_menu);
        if (create_menu) {
            ImGui::InputText("Menu Name", menu_name, sizeof(menu_name));
            if (ImGui::Button("Create New Menu")) {
                show_menu = true;
                create_menu = false;
            }
        }

        ImGui::End();

        // Element Tree Window
        if (show_element_tree) {
            ImGui::SetNextWindowPos(ImVec2(10, 150), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
            ImGui::Begin("Element Tree", &show_element_tree);

            RenderElementTree();

            ImGui::End();
        }

        // Properties Window
        if (show_properties) {
            ImGui::SetNextWindowPos(ImVec2(320, 150), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);
            ImGui::Begin("Properties", &show_properties);

            RenderProperties();

            ImGui::End();
        }

        // Preview Window
        if (show_preview && show_menu) {
            ImGui::SetNextWindowPos(ImVec2(680, 30), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);
            ImGui::Begin(menu_name, &show_menu);

            RenderPreview();

            ImGui::End();
        }
    }

private:
    void RenderElementTree() {
        ImGui::Text("ImGui Elements Library");
        ImGui::Separator();

        // Element categories
        if (ImGui::TreeNode("Basic Elements")) {
            AddElementButton("Button", ElementType::BUTTON);
            AddElementButton("Checkbox", ElementType::CHECKBOX);
            AddElementButton("Text", ElementType::TEXT);
            AddElementButton("Separator", ElementType::SEPARATOR);
            AddElementButton("Spacing", ElementType::SPACING);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Input Elements")) {
            AddElementButton("Text Input", ElementType::INPUT_TEXT);
            AddElementButton("Integer Input", ElementType::INPUT_INT);
            AddElementButton("Float Input", ElementType::INPUT_FLOAT);
            AddElementButton("Slider Float", ElementType::SLIDER_FLOAT);
            AddElementButton("Slider Int", ElementType::SLIDER_INT);
            AddElementButton("Color Picker", ElementType::COLOR_PICKER);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Selection Elements")) {
            AddElementButton("Combo Box", ElementType::COMBO);
            AddElementButton("List Box", ElementType::LISTBOX);
            AddElementButton("Radio Button", ElementType::RADIO_BUTTON);
            AddElementButton("Selectable", ElementType::SELECTABLE);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Layout Elements")) {
            AddElementButton("Tree Node", ElementType::TREE_NODE);
            AddElementButton("Collapsing Header", ElementType::COLLAPSING_HEADER);
            AddElementButton("Tab Bar", ElementType::TAB_BAR);
            AddElementButton("Tab Item", ElementType::TAB_ITEM);
            AddElementButton("Child Window", ElementType::CHILD_WINDOW);
            AddElementButton("Columns", ElementType::COLUMNS);
            AddElementButton("Table", ElementType::TABLE);
            AddElementButton("Group", ElementType::GROUP);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Menu Elements")) {
            AddElementButton("Menu Bar", ElementType::MENU_BAR);
            AddElementButton("Menu Item", ElementType::MENU_ITEM);
            AddElementButton("Popup", ElementType::POPUP);
            AddElementButton("Tooltip", ElementType::TOOLTIP);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Visual Elements")) {
          
            AddElementButton("Bullet Text", ElementType::BULLET_TEXT);
            AddElementButton("Plot Lines", ElementType::PLOT_LINES);
            AddElementButton("Plot Histogram", ElementType::PLOT_HISTOGRAM);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Utility Elements")) {
            AddElementButton("Same Line", ElementType::SAME_LINE);
            AddElementButton("New Line", ElementType::NEW_LINE);
            AddElementButton("Indent", ElementType::INDENT);
            AddElementButton("Unindent", ElementType::UNINDENT);
            ImGui::TreePop();
        }

        ImGui::Separator();
        ImGui::Text("Created Elements:");

        for (auto& element : elements) {
            RenderElementInTree(element);
        }

        if (ImGui::Button("Clear All Elements")) {
            elements.clear();
            selected_element = nullptr;
        }
    }

    void AddElementButton(const char* name, ElementType type) {
        if (ImGui::Button(name)) {
            auto element = std::make_shared<ImGuiElement>(type, name);

            // Set default values based on type
            switch (type) {
            case ElementType::COMBO:
                element->combo_items = { "Option 1", "Option 2", "Option 3" };
                break;
            case ElementType::SLIDER_FLOAT:
                element->min_value = 0.0f;
                element->max_value = 100.0f;
                element->float_value = 50.0f;
                break;
            case ElementType::SLIDER_INT:
                element->min_value = 0;
                element->max_value = 100;
                element->int_value = 50;
                break;
            case ElementType::PROGRESS_BAR:
                element->float_value = 0.5f;
                break;
            case ElementType::TEXT:
                element->text_value = "Sample Text";
                break;
            case ElementType::INPUT_TEXT:
                element->text_value = "Enter text...";
                break;
            }

            elements.push_back(element);
            selected_element = element;
        }
    }

    void RenderElementInTree(std::shared_ptr<ImGuiElement> element) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (element == selected_element) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        bool node_open = ImGui::TreeNodeEx(element->label.c_str(), flags);

        if (ImGui::IsItemClicked()) {
            selected_element = element;
        }

        // Context menu
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete")) {
                auto it = std::find(elements.begin(), elements.end(), element);
                if (it != elements.end()) {
                    elements.erase(it);
                    if (selected_element == element) {
                        selected_element = nullptr;
                    }
                }
            }
            if (ImGui::MenuItem("Duplicate")) {
                auto new_element = std::make_shared<ImGuiElement>(*element);
                new_element->label += " Copy";
                new_element->id = new_element->label + "##" + std::to_string(rand());
                elements.push_back(new_element);
            }
            ImGui::EndPopup();
        }

        if (node_open) {
            for (auto& child : element->children) {
                RenderElementInTree(child);
            }
            ImGui::TreePop();
        }
    }

    void RenderProperties() {
        if (!selected_element) {
            ImGui::Text("No element selected");
            ImGui::Text("Select an element from the tree to edit its properties");
            return;
        }

        ImGui::Text("Element Properties");
        ImGui::Separator();

        // Basic properties
        char label_buffer[256];
        strcpy_s(label_buffer, selected_element->label.c_str());
        if (ImGui::InputText("Label", label_buffer, sizeof(label_buffer))) {
            selected_element->label = label_buffer;
        }

        ImGui::Checkbox("Enabled", &selected_element->enabled);
        ImGui::Checkbox("Visible", &selected_element->visible);

        // Type-specific properties
        switch (selected_element->type) {
        case ElementType::CHECKBOX:
            ImGui::Checkbox("Default Value", &selected_element->bool_value);
            break;

        case ElementType::SLIDER_FLOAT:
            ImGui::DragFloat("Min Value", &selected_element->min_value);
            ImGui::DragFloat("Max Value", &selected_element->max_value);
            ImGui::DragFloat("Default Value", &selected_element->float_value, 1.0f, selected_element->min_value, selected_element->max_value);
            break;

        case ElementType::SLIDER_INT:
            ImGui::DragFloat("Min Value", &selected_element->min_value);
            ImGui::DragFloat("Max Value", &selected_element->max_value);
            ImGui::DragInt("Default Value", &selected_element->int_value, 1.0f, (int)selected_element->min_value, (int)selected_element->max_value);
            break;

        case ElementType::INPUT_TEXT:
        case ElementType::TEXT:
        case ElementType::BULLET_TEXT:
            char text_buffer[1024];
            strcpy_s(text_buffer, selected_element->text_value.c_str());
            if (ImGui::InputTextMultiline("Text Content", text_buffer, sizeof(text_buffer))) {
                selected_element->text_value = text_buffer;
            }
            break;

        case ElementType::COMBO:
        case ElementType::LISTBOX:
            ImGui::Text("Combo Items:");
            for (size_t i = 0; i < selected_element->combo_items.size(); ++i) {
                char item_buffer[256];
                strcpy_s(item_buffer, selected_element->combo_items[i].c_str());
                ImGui::PushID((int)i);
                if (ImGui::InputText("##item", item_buffer, sizeof(item_buffer))) {
                    selected_element->combo_items[i] = item_buffer;
                }
                ImGui::SameLine();
                if (ImGui::Button("X")) {
                    selected_element->combo_items.erase(selected_element->combo_items.begin() + i);
                    --i;
                }
                ImGui::PopID();
            }
            if (ImGui::Button("Add Item")) {
                selected_element->combo_items.push_back("New Item");
            }
            break;

        case ElementType::COLOR_PICKER:
            ImGui::ColorEdit4("Default Color", (float*)&selected_element->color_value);
            break;

        case ElementType::PROGRESS_BAR:
            ImGui::SliderFloat("Progress", &selected_element->float_value, 0.0f, 1.0f);
            break;
        }

        // Style properties
        ImGui::Separator();
        ImGui::Text("Style Properties");

        ImGui::DragFloat2("Size", (float*)&selected_element->size);
        ImGui::ColorEdit4("Text Color", (float*)&selected_element->text_color);
        ImGui::ColorEdit4("Background Color", (float*)&selected_element->bg_color);

        // Code generation
        ImGui::Separator();
        if (ImGui::Button("Generate Code")) {
            // This would generate C++ ImGui code
            ImGui::OpenPopup("Generated Code");
        }

        if (ImGui::BeginPopupModal("Generated Code", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Generated C++ Code:");
            ImGui::Separator();

            std::string code = GenerateCodeForElement(selected_element);
            ImGui::TextWrapped("%s", code.c_str());

            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void RenderPreview() {
       
        

        for (auto& element : elements) {
            RenderElementPreview(element);
        }
    }

    void RenderElementPreview(std::shared_ptr<ImGuiElement> element) {
        if (!element->visible) return;

        // Apply styling
        if (element->size.x > 0 || element->size.y > 0) {
            ImGui::PushItemWidth(element->size.x);
        }

        ImGui::PushStyleColor(ImGuiCol_Text, element->text_color);

        switch (element->type) {
        case ElementType::BUTTON:
            if (ImGui::Button(element->label.c_str(), element->size)) {
                // Button clicked
            }
            break;

        case ElementType::CHECKBOX:
            ImGui::Checkbox(element->label.c_str(), &element->bool_value);
            break;

        case ElementType::SLIDER_FLOAT:
            ImGui::SliderFloat(element->label.c_str(), &element->float_value, element->min_value, element->max_value);
            break;

        case ElementType::SLIDER_INT:
            ImGui::SliderInt(element->label.c_str(), &element->int_value, (int)element->min_value, (int)element->max_value);
            break;

        case ElementType::INPUT_TEXT:
        {
            static char buffer[1024];
            strcpy_s(buffer, element->text_value.c_str());
            if (ImGui::InputText(element->label.c_str(), buffer, sizeof(buffer))) {
                element->text_value = buffer;
            }
        }
        break;

        case ElementType::INPUT_INT:
            ImGui::InputInt(element->label.c_str(), &element->int_value);
            break;

        case ElementType::INPUT_FLOAT:
            ImGui::InputFloat(element->label.c_str(), &element->float_value);
            break;

        case ElementType::COMBO:
            if (!element->combo_items.empty()) {
                const char* current_item = element->combo_items[element->selected_item].c_str();
                if (ImGui::BeginCombo(element->label.c_str(), current_item)) {
                    for (size_t i = 0; i < element->combo_items.size(); ++i) {
                        bool is_selected = (element->selected_item == (int)i);
                        if (ImGui::Selectable(element->combo_items[i].c_str(), is_selected)) {
                            element->selected_item = (int)i;
                        }
                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            break;

        case ElementType::LISTBOX:
            if (!element->combo_items.empty()) {
                std::vector<const char*> items;
                for (const auto& item : element->combo_items) {
                    items.push_back(item.c_str());
                }
                ImGui::ListBox(element->label.c_str(), &element->selected_item, items.data(), (int)items.size());
            }
            break;

        case ElementType::COLOR_PICKER:
            ImGui::ColorEdit4(element->label.c_str(), (float*)&element->color_value);
            break;

        case ElementType::SEPARATOR:
            ImGui::Separator();
            break;

        case ElementType::TEXT:
            ImGui::Text("%s", element->text_value.c_str());
            break;

        case ElementType::BULLET_TEXT:
            ImGui::BulletText("%s", element->text_value.c_str());
            break;

        case ElementType::TREE_NODE:
            if (ImGui::TreeNode(element->label.c_str())) {
                for (auto& child : element->children) {
                    RenderElementPreview(child);
                }
                ImGui::TreePop();
            }
            break;

        case ElementType::COLLAPSING_HEADER:
            if (ImGui::CollapsingHeader(element->label.c_str())) {
                for (auto& child : element->children) {
                    RenderElementPreview(child);
                }
            }
            break;

        case ElementType::PROGRESS_BAR:
            ImGui::ProgressBar(element->float_value, element->size, element->label.c_str());
            break;

        case ElementType::RADIO_BUTTON:
            ImGui::RadioButton(element->label.c_str(), &element->int_value, 1);
            break;

        case ElementType::SELECTABLE:
            ImGui::Selectable(element->label.c_str(), &element->bool_value);
            break;

        case ElementType::SPACING:
            ImGui::Spacing();
            break;

        case ElementType::SAME_LINE:
            ImGui::SameLine();
            break;

        case ElementType::NEW_LINE:
            ImGui::NewLine();
            break;

        case ElementType::INDENT:
            ImGui::Indent();
            break;

        case ElementType::UNINDENT:
            ImGui::Unindent();
            break;
        }

        ImGui::PopStyleColor();

        if (element->size.x > 0 || element->size.y > 0) {
            ImGui::PopItemWidth();
        }
    }

    std::string GenerateCodeForElement(std::shared_ptr<ImGuiElement> element) {
        std::string code;

        switch (element->type) {
        case ElementType::BUTTON:
            code = "if (ImGui::Button(\"" + element->label + "\")) {\n    // Button clicked\n}";
            break;
        case ElementType::CHECKBOX:
            code = "static bool " + element->id + " = " + (element->bool_value ? "true" : "false") + ";\n";
            code += "ImGui::Checkbox(\"" + element->label + "\", &" + element->id + ");";
            break;
        case ElementType::SLIDER_FLOAT:
            code = "static float " + element->id + " = " + std::to_string(element->float_value) + "f;\n";
            code += "ImGui::SliderFloat(\"" + element->label + "\", &" + element->id + ", " +
                std::to_string(element->min_value) + "f, " + std::to_string(element->max_value) + "f);";
            break;
        case ElementType::TEXT:
            code = "ImGui::Text(\"" + element->text_value + "\");";
            break;
        default:
            code = "// Code generation for this element type not implemented yet";
            break;
        }

        return code;
    }
};

// Global builder instance
ImGuiBuilder g_builder;

// Main code
int main(int, char**)
{
    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Builder", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"?? ULTIMATE ImGui Builder - Create Amazing Menus!", WS_OVERLAPPEDWINDOW, 100, 100, 1400, 900, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
   

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Customize colors for better look
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.9f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.4f, 0.8f, 0.8f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.5f, 0.9f, 0.8f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.4f, 0.6f, 1.0f, 0.8f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.4f, 0.8f, 0.6f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.5f, 0.9f, 0.8f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.4f, 0.6f, 1.0f, 1.0f);

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // Our state
    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle lost D3D9 device
        if (g_DeviceLost)
        {
            HRESULT hr = g_pd3dDevice->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST)
            {
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET)
                ResetDevice();
            g_DeviceLost = false;
        }

        // Handle window resize
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Enable docking
       

        // Render our builder
        g_builder.Render();

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
        if (result == D3DERR_DEVICELOST)
            g_DeviceLost = true;
    }

    // Cleanup
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions
bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
