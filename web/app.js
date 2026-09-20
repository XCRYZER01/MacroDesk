(function () {
  "use strict";

  const STORAGE_KEY = "macrodesk-studio-project-v3";
  const STORAGE_KEY_V2 = "macrodesk-studio-project-v2";
  const BUILT_IN_BACKGROUNDS = new Set([
    "../firmware/MacroDeckUI/assets/ui_orca_clean_800x480.png",
    "../firmware/MacroDeckUI/assets/ui_prusa_clean_800x480.png",
    "../firmware/MacroDeckUI/assets/ui_fusion_clean_800x480.png",
    "../firmware/MacroDeckUI/assets/ui_onshape_clean_800x480.png",
    "../firmware/MacroDeckUI/assets/ui_blender_clean_800x480.png"
  ]);
  const ICONS = ["S", "↶", "↷", "＋", "−", "⌂", "◫", "✂", "◉", "↔", "◇", "⚙"];
  const VALID_ACTIONS = new Set(["keys", "search", "text", "page", "none"]);
  const AREA_NAMES = { main: "Main grid", sidebar: "Left sidebar", panel: "Right sidebar", quick: "Quick action" };

  const appTemplates = {
    orca: makeTemplate({
      id: "orca", name: "OrcaSlicer", badge: "O", accent: "#22c7bd",
      backgroundImage: "../firmware/MacroDeckUI/assets/ui_orca_clean_800x480.png",
      panelTitle: "VIEW",
      pages: ["Prepare", "Modify", "View", "Support", "Filament", "Printer", "Tools", "Settings"],
      main: [
        ["New", "Ctrl+N", "＋"], ["Open", "Ctrl+O", "◫"], ["Save", "Ctrl+S", "S"], ["Import", "Ctrl+I", "＋"], ["Arrange", "A", "◇"], ["Orient", "Q", "◉"],
        ["Instance +", "+", "＋"], ["Instance −", "-", "−"], ["Move", "M", "↔"], ["Rotate", "R", "↷"], ["Scale", "S", "◇"], ["Lay Flat", "F", "⌂"],
        ["Cut", "C", "✂"], ["Support Paint", "L", "S"], ["Seam Paint", "P", "P"], ["Fuzzy Skin", "H", "~"], ["Color Paint", "N", "◉"], ["Add Text", "T", "T"],
        ["Measure", "U", "↔"], ["Undo", "Ctrl+Z", "↶"], ["Redo", "Ctrl+Y", "↷"], ["Delete", "Del", "✂"], ["Clone", "Ctrl+K", "◫"]
      ],
      quick: ["Slice Plate", "Ctrl+R", "S"],
      panel: [
        ["Default", "Ctrl+0", "0"], ["Top", "Ctrl+1", "1"], ["Bottom", "Ctrl+2", "2"],
        ["Front", "Ctrl+3", "3"], ["Behind", "Ctrl+4", "4"], ["Left", "Ctrl+5", "5"],
        ["Right", "Ctrl+6", "6"], ["Prep/Preview", "Tab", "P"]
      ]
    }),
    prusa: makeTemplate({
      id: "prusa", name: "PrusaSlicer", badge: "P", accent: "#f26b38",
      backgroundImage: "../firmware/MacroDeckUI/assets/ui_prusa_clean_800x480.png",
      panelTitle: "VIEW",
      pages: ["Plater", "Project", "Transform", "View", "Preview", "Export", "Tools", "Settings"],
      main: [
        ["New Project", "Ctrl+N", "＋"], ["Open Project", "Ctrl+O", "◫"], ["Save Project", "Ctrl+S", "S"], ["Save As", "Ctrl+Shift+S", "S"], ["Import Model", "Ctrl+I", "＋"], ["Load Config", "Ctrl+L", "◫"],
        ["Slice", "Ctrl+R", "S"], ["Export G-code", "Ctrl+G", "◫"], ["Export Config", "Ctrl+E", "◫"], ["Export SD", "Ctrl+U", "◫"], ["Eject Drive", "Ctrl+T", "−"], ["Select All", "Ctrl+A", "◇"],
        ["Deselect", "Esc", "−"], ["Delete", "Del", "✂"], ["Delete All", "Ctrl+Del", "✂"], ["Undo", "Ctrl+Z", "↶"], ["Redo", "Ctrl+Y", "↷"], ["Copy", "Ctrl+C", "◫"],
        ["Paste", "Ctrl+V", "◫"], ["Arrange", "A", "◇"], ["Instance +", "+", "＋"], ["Instance −", "-", "−"], ["Cut", "C", "✂"], ["Place on Face", "F", "⌂"]
      ],
      quick: ["Slice", "Ctrl+R", "S"],
      panel: [
        ["Move", "M", "↔"], ["Scale", "S", "◇"], ["Rotate", "R", "↷"],
        ["3D View", "Ctrl+5", "3"], ["Preview", "Ctrl+6", "P"], ["Zoom Bed", "B", "⌂"],
        ["Zoom All", "Z", "◫"], ["Sidebar", "Shift+Tab", "S"], ["Camera", "K", "◉"]
      ]
    }),
    fusion: makeTemplate({
      id: "fusion", name: "Fusion 360", badge: "F", accent: "#3a9bff",
      backgroundImage: "../firmware/MacroDeckUI/assets/ui_fusion_clean_800x480.png",
      panelTitle: "VIEW", showQuickAction: false,
      pages: ["Home", "Sketch", "Solid", "Surface", "Mesh", "Sheet Metal", "Tools", "Settings"],
      main: [
        ["New Design", "Ctrl+N", "＋"], ["Open", "Ctrl+O", "◫"], ["Save", "Ctrl+S", "S"], ["Undo", "Ctrl+Z", "↶"], ["Redo", "Ctrl+Y", "↷"], ["Line", "L", "↔"],
        ["Rectangle", "R", "◇"], ["Circle", "C", "◉"], ["Arc", "3-Point Arc", ")", "search"], ["Dimension", "D", "↔"], ["Extrude", "E", "＋"], ["Revolve", "Revolve", "↷", "search"],
        ["Fillet", "F", "⌒"], ["Chamfer", "Chamfer", "/", "search"], ["Shell", "Shell", "◇", "search"], ["Move", "M", "↔"], ["Combine", "Combine", "＋", "search"], ["Hole", "H", "◉"],
        ["Pattern", "Rectangular Pattern", "::", "search"], ["Mirror", "Mirror", "|", "search"]
      ],
      quick: ["Fit", "F6", "⌂"],
      panel: [
        ["Home", "", "⌂", "none"], ["Fit", "F6", "⌂"], ["Visibility", "V", "◉"],
        ["Full Screen", "Ctrl+Shift+F", "◫"], ["4 Views", "Shift+1", "4"], ["Shaded", "Ctrl+4", "S"],
        ["Wireframe", "Ctrl+7", "W"], ["Hidden Edges", "Ctrl+5", "H"]
      ]
    }),
    blender: makeTemplate({
      id: "blender", name: "Blender", badge: "B", accent: "#f29a2e",
      backgroundImage: "../firmware/MacroDeckUI/assets/ui_blender_clean_800x480.png",
      panelTitle: "VIEW",
      pages: ["Layout", "Model", "Sculpt", "UV", "Material", "Animate", "Render", "Settings"],
      main: [
        ["Save", "Ctrl+S", "S"], ["Save As", "Ctrl+Shift+S", "S"], ["Open", "Ctrl+O", "◫"], ["Undo", "Ctrl+Z", "↶"], ["Redo", "Ctrl+Shift+Z", "↷"], ["Search", "F3", "⌕"],
        ["Move", "G", "↔"], ["Rotate", "R", "↷"], ["Scale", "S", "◇"], ["Add", "Shift+A", "＋"], ["Delete", "X", "✂"], ["Duplicate", "Shift+D", "◫"],
        ["Edit Mode", "Tab", "◇"], ["Extrude", "E", "＋"], ["Inset", "I", "◇"], ["Bevel", "Ctrl+B", "⌒"], ["Loop Cut", "Ctrl+R", "✂"], ["Last Op", "F9", "⚙"],
        ["Join", "Ctrl+J", "＋"], ["Apply", "Ctrl+A", "◉"], ["Parent", "Ctrl+P", "◫"], ["Hide", "H", "−"], ["Unhide", "Alt+H", "◉"], ["Play", "Space", "▶"]
      ],
      quick: ["Render", "F12", "◉"],
      panel: [
        ["Frame All", "Home", "⌂"], ["X-Ray", "Alt+Z", "◫"], ["Wireframe", "Shift+Z", "W"],
        ["Shading", "Z", "◉"], ["Quad View", "Ctrl+Alt+Q", "4"], ["Sidebar", "N", "◫"],
        ["Toolbar", "T", "T"], ["Render", "F12", "◉"], ["Render Anim", "Ctrl+F12", "▶"]
      ]
    }),
    onshape: makeTemplate({
      id: "onshape", name: "Onshape", badge: "N", accent: "#56c596",
      backgroundImage: "../firmware/MacroDeckUI/assets/ui_onshape_clean_800x480.png",
      panelTitle: "VIEW",
      pages: ["Part Studio", "Sketch", "Features", "Assembly", "View", "Measure", "Tools", "Settings"],
      main: [
        ["Undo", "Ctrl+Z", "↶"], ["Redo", "Ctrl+Y", "↷"], ["Search Tools", "Alt+C", "⌕"], ["New Sketch", "Shift+S", "＋"], ["Extrude", "Shift+E", "＋"], ["Revolve", "Shift+W", "↷"],
        ["Fillet", "Shift+F", "⌒"], ["Line", "L", "↔"], ["Rectangle", "R", "◇"], ["Circle", "C", "◉"], ["Dimension", "D", "↔"], ["Zoom to Fit", "F", "⌂"],
        ["3-Point Arc", "A", ")"], ["Corner Rect", "G", "◇"], ["Trim", "M", "✂"], ["Offset", "O", "◫"], ["Use/Project", "U", "＋"], ["Construction", "Q", "◇"],
        ["Copy", "Ctrl+C", "◫"], ["Paste", "Ctrl+V", "◫"], ["Delete", "Del", "✂"], ["Show Sketches", "Shift+H", "◉"], ["Planes", "P", "◫"], ["Measure", "[", "↔"]
      ],
      quick: ["Search Tools", "Alt+C", "⌕"],
      panel: [
        ["Front", "Shift+1", "1"], ["Back", "Shift+2", "2"], ["Left", "Shift+3", "3"],
        ["Right", "Shift+4", "4"], ["Top", "Shift+5", "5"], ["Bottom", "Shift+6", "6"],
        ["Isometric", "Shift+7", "7"], ["Zoom Fit", "F", "⌂"], ["Section", "Shift+X", "✂"]
      ]
    })
  };

  function makeTemplate(config) {
    return {
      id: config.id, name: config.name, badge: config.badge, accent: config.accent,
      backgroundImage: config.backgroundImage || null,
      backgroundDim: config.backgroundImage ? 0 : 0.28,
      panelTitle: config.panelTitle || "PANEL",
      panelColumns: 3,
      panelRows: 3,
      showQuickAction: config.showQuickAction !== false,
      pages: config.pages,
      sidebarButtons: makeButtons(
        config.id,
        "sidebar",
        config.pages.map((label, index) => [label, String(index + 1), label.slice(0, 1).toUpperCase(), "page"]),
        8,
        config.accent
      ),
      // One deck per sidebar page, like the firmware's sidebar sub-pages.
      // The template fills page 1; the rest are the user's to build.
      activePage: 0,
      pageButtons: config.pages.map((_, page) => makeButtons(
        config.id, `page${page + 1}`, page === 0 ? config.main : [], mainCount(config.id), config.accent
      )),
      panelButtons: makeButtons(config.id, "panel", config.panel, 9, config.accent),
      quickAction: makeButtons(config.id, "quick", [config.quick], 1, config.accent)[0]
    };
  }

  // Declared, not assigned, because the templates above call it while they are built.
  function mainCount(profileId) { return profileId === "fusion" ? 20 : 24; }

  function makeButtons(profileId, area, definitions, count, accent) {
    return Array.from({ length: count }, (_, index) => {
      const item = definitions[index] || ["Empty", "", "−", "none"];
      return {
        id: `${profileId}-${area}-${index + 1}`, label: item[0],
        action: item[3] || "keys", value: item[1], icon: item[2], iconImage: null,
        color: accent, enabled: item[3] !== "none"
      };
    });
  }

  const clone = value => JSON.parse(JSON.stringify(value));
  const $ = selector => document.querySelector(selector);

  function makeProject(first = "orca", second = "fusion") {
    return {
      format: "macrodesk-profile", version: 3,
      device: { model: "esp32-s3-touch-lcd-7", width: 800, height: 480, layout: "deck-template-v1" },
      activeProfile: 0, profiles: [clone(appTemplates[first]), clone(appTemplates[second])]
    };
  }

  let project = loadProject();
  let selection = { area: "main", index: 0 };
  let draggedButton = null;
  let saveTimer = null;
  let toastTimer = null;

  const elements = {
    profileOne: $("#profileOne"), profileTwo: $("#profileTwo"), buttonList: $("#buttonList"),
    buttonListTitle: $("#buttonListTitle"), areaTabs: $("#areaTabs"), pageStrip: $("#pageStrip"), previewTitle: $("#previewTitle"),
    screenProfileName: $("#screenProfileName"), appBadge: $("#appBadge"), deviceScreen: $("#deviceScreen"),
    screenGrid: $("#screenGrid"), screenPanelGrid: $("#screenPanelGrid"), screenQuick: $("#screenQuick"),
    screenSidebar: $("#screenSidebar"), screenTabs: $("#screenTabs"), quickAction: $("#quickAction"),
    quickShortcut: $("#quickShortcut"), inspectorFields: $("#inspectorFields"),
    selectionPosition: $("#selectionPosition"), buttonLabel: $("#buttonLabel"), buttonAction: $("#buttonAction"),
    buttonValue: $("#buttonValue"), valueLabel: $("#valueLabel"), valueHelp: $("#valueHelp"),
    iconChoices: $("#iconChoices"), buttonColor: $("#buttonColor"), colorValue: $("#colorValue"),
    buttonEnabled: $("#buttonEnabled"), clearButton: $("#clearButton"), importFile: $("#importFile"),
    iconFile: $("#iconFile"), backgroundFile: $("#backgroundFile"), backgroundDim: $("#backgroundDim"),
    backgroundDimValue: $("#backgroundDimValue"), removeBackground: $("#removeBackground"),
    panelTitle: $("#panelTitle"), panelColumns: $("#panelColumns"), panelRows: $("#panelRows"),
    showQuickAction: $("#showQuickAction"),
    saveState: $("#saveState"), toast: $("#toast")
  };

  function init() {
    const options = Object.values(appTemplates).map(item => `<option value="${item.id}">${item.name}</option>`).join("");
    elements.profileOne.innerHTML = options;
    elements.profileTwo.innerHTML = options;
    ICONS.forEach(icon => {
      const button = document.createElement("button");
      button.className = "icon-choice"; button.type = "button"; button.textContent = icon; button.title = `Use ${icon}`;
      button.addEventListener("click", () => updateButton({ icon, iconImage: null }));
      elements.iconChoices.appendChild(button);
    });
    bindEvents();
    render();
  }

  function bindEvents() {
    elements.profileOne.addEventListener("change", event => replaceProfile(0, event.target.value));
    elements.profileTwo.addEventListener("change", event => replaceProfile(1, event.target.value));
    elements.areaTabs.addEventListener("click", event => {
      const button = event.target.closest("button[data-area]");
      if (button) selectButton(button.dataset.area, 0);
    });
    elements.buttonLabel.addEventListener("input", event => updateButton({ label: event.target.value }));
    elements.buttonAction.addEventListener("change", event => updateButton({ action: event.target.value }));
    elements.buttonValue.addEventListener("input", event => updateButton({ value: event.target.value }));
    elements.buttonColor.addEventListener("input", event => updateButton({ color: event.target.value.toUpperCase() }));
    elements.buttonEnabled.addEventListener("change", event => updateButton({ enabled: event.target.checked }));
    $("#uploadIconButton").addEventListener("click", () => elements.iconFile.click());
    elements.iconFile.addEventListener("change", importIcon);
    $("#uploadBackgroundButton").addEventListener("click", () => elements.backgroundFile.click());
    elements.backgroundFile.addEventListener("change", importBackground);
    elements.removeBackground.addEventListener("click", () => { currentProfile().backgroundImage = null; changed("Background removed"); });
    elements.backgroundDim.addEventListener("input", event => { currentProfile().backgroundDim = Number(event.target.value) / 100; changed(); });
    elements.panelTitle.addEventListener("input", event => { currentProfile().panelTitle = event.target.value; changed(); });
    elements.panelColumns.addEventListener("change", event => { currentProfile().panelColumns = Number(event.target.value); selection = { area: "panel", index: 0 }; changed("Right sidebar layout updated"); });
    elements.panelRows.addEventListener("change", event => { currentProfile().panelRows = Number(event.target.value); selection = { area: "panel", index: 0 }; changed("Right sidebar layout updated"); });
    elements.showQuickAction.addEventListener("change", event => { currentProfile().showQuickAction = event.target.checked; selection = { area: event.target.checked ? "quick" : "panel", index: 0 }; changed("Right sidebar layout updated"); });
    elements.clearButton.addEventListener("click", () => updateButton({ label: "Empty", action: "none", value: "", icon: "−", iconImage: null, enabled: false }));
    $("#resetProfile").addEventListener("click", resetCurrentProfile);
    $("#exportButton").addEventListener("click", exportProject);
    $("#importButton").addEventListener("click", () => elements.importFile.click());
    elements.importFile.addEventListener("change", importProject);
  }

  const currentProfile = () => project.profiles[project.activeProfile];
  function buttonsFor(profile, area = selection.area) {
    if (area === "main") return profile.pageButtons[profile.activePage];
    if (area === "sidebar") return profile.sidebarButtons;
    if (area === "panel") return profile.panelButtons.slice(0, panelSlotCount(profile));
    return [profile.quickAction];
  }

  // A sidebar key opens the page its own value points at, so the preview follows
  // whatever target the user typed rather than the key's position.
  function pageTarget(button, index) {
    if (button.action !== "page") return index;
    const page = Number.parseInt(button.value, 10);
    return Number.isFinite(page) ? Math.min(8, Math.max(1, page)) - 1 : index;
  }

  function pageName(profile, index) {
    const owner = profile.sidebarButtons.find((button, i) => pageTarget(button, i) === index && button.action === "page");
    return (owner && owner.label) || profile.pages[index] || `Page ${index + 1}`;
  }

  function openPage(index) {
    const profile = currentProfile();
    if (profile.activePage === index && selection.area === "main") return;
    profile.activePage = index;
    selection = { area: "main", index: 0 };
    changed();
  }
  const panelSlotCount = profile => profile.panelColumns * profile.panelRows;
  const currentButton = () => buttonsFor(currentProfile())[selection.index] || null;

  function replaceProfile(index, templateId) {
    project.profiles[index] = clone(appTemplates[templateId]);
    project.activeProfile = index; selection = { area: "main", index: 0 };
    changed(`${appTemplates[templateId].name} loaded`);
  }

  function resetCurrentProfile() {
    const source = appTemplates[currentProfile().id];
    if (!source) return;
    project.profiles[project.activeProfile] = clone(source);
    selection = { area: "main", index: 0 };
    changed(`${source.name} reset to its template`);
  }

  function selectButton(area, index) {
    selection = { area, index };
    render();
  }

  function updateButton(patch) {
    const button = currentButton();
    if (!button) return;
    Object.assign(button, patch);
    if (!button.enabled && button.action !== "none" && ("action" in patch || "value" in patch)) button.enabled = true;
    changed();
  }

  function changed(message) {
    elements.saveState.textContent = "Saving…";
    clearTimeout(saveTimer);
    saveTimer = setTimeout(() => {
      try {
        localStorage.setItem(STORAGE_KEY, JSON.stringify(project));
        elements.saveState.textContent = "Saved locally";
      } catch (_) {
        elements.saveState.textContent = "Storage full — export your file";
      }
    }, 180);
    render();
    if (message) showToast(message);
  }

  function render() {
    const profile = currentProfile();
    elements.profileOne.value = project.profiles[0].id;
    elements.profileTwo.value = project.profiles[1].id;
    elements.previewTitle.textContent = `${profile.name} · deck template`;
    elements.buttonListTitle.textContent = selection.area === "main"
      ? `${pageName(profile, profile.activePage)} · page ${profile.activePage + 1}`
      : `${profile.name} · ${AREA_NAMES[selection.area]}`;
    elements.screenProfileName.textContent = profile.name;
    elements.appBadge.textContent = profile.badge;
    elements.deviceScreen.style.setProperty("--profile-accent", profile.accent);
    elements.deviceScreen.style.setProperty("--main-columns", profile.id === "fusion" ? 5 : 6);
    elements.deviceScreen.style.setProperty("--panel-columns", profile.panelColumns);
    elements.deviceScreen.style.setProperty("--panel-rows", profile.panelRows);
    elements.deviceScreen.classList.toggle("legacy-art-layout", profile.id === "orca" || profile.id === "fusion" || profile.id === "onshape" || profile.id === "blender");
    elements.deviceScreen.classList.toggle("full-right-panel", !profile.showQuickAction);
    elements.panelTitle.value = profile.panelTitle;
    elements.panelColumns.value = String(profile.panelColumns);
    elements.panelRows.value = String(profile.panelRows);
    elements.showQuickAction.checked = profile.showQuickAction;
    elements.areaTabs.querySelector('[data-area="main"]').textContent = `Page ${profile.activePage + 1} · ${buttonsFor(profile, "main").length}`;
    elements.areaTabs.querySelector('[data-area="sidebar"]').textContent = `Left sidebar ${profile.sidebarButtons.length}`;
    elements.areaTabs.querySelector('[data-area="panel"]').textContent = `Right sidebar ${panelSlotCount(profile)}`;
    elements.areaTabs.querySelector('[data-area="quick"]').hidden = !profile.showQuickAction;
    elements.areaTabs.querySelectorAll("button").forEach(button => button.classList.toggle("active", button.dataset.area === selection.area));
    renderBackground(profile);
    renderPageStrip(profile);
    renderButtonList(profile);
    renderPreview(profile);
    renderInspector(currentButton());
  }

  function renderBackground(profile) {
    const dim = Math.min(0.8, Math.max(0, profile.backgroundDim));
    elements.backgroundDim.value = String(Math.round(dim * 100));
    elements.backgroundDimValue.textContent = `${Math.round(dim * 100)}%`;
    elements.removeBackground.disabled = !profile.backgroundImage;
    elements.deviceScreen.style.backgroundImage = profile.backgroundImage
      ? `linear-gradient(rgba(7, 17, 22, ${dim}), rgba(7, 17, 22, ${dim})), url("${profile.backgroundImage}")`
      : "linear-gradient(rgba(255,255,255,.018) 1px, transparent 1px), linear-gradient(90deg, rgba(255,255,255,.018) 1px, transparent 1px)";
    elements.deviceScreen.style.backgroundSize = profile.backgroundImage ? "cover" : "20px 20px";
    elements.deviceScreen.style.backgroundPosition = "center";
  }

  function renderPageStrip(profile) {
    elements.pageStrip.innerHTML = "";
    profile.pageButtons.forEach((buttons, index) => {
      const used = buttons.filter(button => button.enabled).length;
      const chip = document.createElement("button");
      chip.type = "button";
      chip.className = `page-chip${index === profile.activePage ? " active" : ""}${used ? "" : " empty"}`;
      chip.textContent = String(index + 1);
      chip.title = `${pageName(profile, index)} — ${used} of ${buttons.length} keys in use`;
      chip.addEventListener("click", () => openPage(index));
      elements.pageStrip.appendChild(chip);
    });
  }

  function renderButtonList(profile) {
    const buttons = buttonsFor(profile);
    elements.buttonList.innerHTML = "";
    buttons.forEach((button, index) => {
      const item = document.createElement("div");
      item.className = `button-list-item${index === selection.index ? " selected" : ""}`;
      item.draggable = buttons.length > 1;
      item.style.setProperty("--button-accent", button.color);
      item.innerHTML = `<span class="handle">⠿</span>${iconMarkup(button, "mini-icon")}<span><strong>${escapeHtml(button.label || "Untitled")}</strong><small>${escapeHtml(actionSummary(button))}</small></span><span class="slot-number">${index + 1}</span>`;
      item.addEventListener("click", () => selectButton(selection.area, index));
      item.addEventListener("dragstart", () => { draggedButton = index; item.classList.add("dragging"); });
      item.addEventListener("dragend", () => { draggedButton = null; item.classList.remove("dragging"); });
      item.addEventListener("dragover", event => event.preventDefault());
      item.addEventListener("drop", event => {
        event.preventDefault();
        if (draggedButton === null || draggedButton === index) return;
        [buttons[draggedButton], buttons[index]] = [buttons[index], buttons[draggedButton]];
        selection.index = index;
        changed("Buttons swapped");
      });
      elements.buttonList.appendChild(item);
    });
  }

  function renderPreview(profile) {
    elements.screenSidebar.innerHTML = "";
    profile.sidebarButtons.forEach((button, index) => {
      const page = document.createElement("button");
      page.type = "button";
      const sidebarSelected = selection.area === "sidebar" && selection.index === index;
      const onThisPage = pageTarget(button, index) === profile.activePage;
      page.className = `${sidebarSelected ? " selected" : ""}${onThisPage ? " active" : ""}${button.enabled ? "" : " disabled"}`.trim();
      page.style.setProperty("--button-accent", button.color);
      page.innerHTML = `${iconMarkup(button, "sidebar-icon")}<strong>${escapeHtml(button.label || "Untitled")}</strong>`;
      page.title = `Open ${button.label || "this page"}`;
      page.addEventListener("click", () => openPage(pageTarget(button, index)));
      elements.screenSidebar.appendChild(page);
    });
    renderPreviewButtons(elements.screenGrid, profile.pageButtons[profile.activePage], "main", "screen-key");
    elements.screenPanelGrid.dataset.panelTitle = profile.panelTitle || "";
    renderPreviewButtons(elements.screenPanelGrid, profile.panelButtons.slice(0, panelSlotCount(profile)), "panel", "screen-panel-key");
    const quick = profile.quickAction;
    elements.quickAction.textContent = quick.label;
    elements.quickShortcut.textContent = actionSummary(quick);
    elements.screenQuick.classList.toggle("selected", selection.area === "quick");
    elements.screenQuick.hidden = !profile.showQuickAction;
    elements.screenQuick.onclick = () => selectButton("quick", 0);
    elements.screenTabs.innerHTML = "";
    project.profiles.forEach((item, index) => {
      const tab = document.createElement("button");
      tab.type = "button"; tab.className = `screen-tab${index === project.activeProfile ? " active" : ""}`; tab.textContent = item.name;
      tab.addEventListener("click", () => { project.activeProfile = index; selection = { area: "main", index: 0 }; changed(); });
      elements.screenTabs.appendChild(tab);
    });
    const systemTab = document.createElement("button");
    systemTab.type = "button"; systemTab.className = "screen-tab system"; systemTab.textContent = "System";
    systemTab.title = "Reserved for device settings";
    elements.screenTabs.appendChild(systemTab);
  }

  function renderPreviewButtons(container, buttons, area, className) {
    container.innerHTML = "";
    buttons.forEach((button, index) => {
      const key = document.createElement("button");
      key.type = "button";
      key.className = `${className}${selection.area === area && selection.index === index ? " selected" : ""}${button.enabled ? "" : " disabled"}`;
      key.style.setProperty("--button-accent", button.color);
      key.innerHTML = `${iconMarkup(button, "key-icon")}<strong>${escapeHtml(button.label || "Untitled")}</strong><small>${escapeHtml(actionSummary(button))}</small>`;
      key.addEventListener("click", () => selectButton(area, index));
      container.appendChild(key);
    });
  }

  function renderInspector(button) {
    elements.inspectorFields.disabled = !button;
    elements.clearButton.disabled = !button;
    if (!button) return;
    elements.selectionPosition.textContent = `${AREA_NAMES[selection.area]} · Key ${selection.index + 1}`;
    elements.buttonLabel.value = button.label;
    elements.buttonAction.value = button.action;
    elements.buttonValue.value = button.value;
    elements.buttonColor.value = button.color;
    elements.colorValue.textContent = button.color.toUpperCase();
    elements.buttonEnabled.checked = button.enabled;
    document.querySelectorAll(".icon-choice").forEach(choice => choice.classList.toggle("active", !button.iconImage && choice.textContent === button.icon));
    const copy = {
      keys: ["Shortcut", "Use Ctrl, Shift, Alt or Win plus a key."],
      search: ["Command name", "The device opens app search, types this name, then presses Enter."],
      text: ["Text", "The device types this text into the active field."],
      page: ["Page", "Choose which device page this sidebar item opens (1–8)."],
      none: ["Value", "This button will not send anything."]
    }[button.action];
    elements.valueLabel.textContent = copy[0]; elements.valueHelp.textContent = copy[1];
    elements.buttonValue.disabled = button.action === "none";
  }

  function actionSummary(button) {
    if (!button.enabled || button.action === "none") return "No action";
    if (button.action === "page") return `Page ${button.value}`;
    if (button.action === "search") return `Search: ${button.value}`;
    if (button.action === "text") return `Type: ${button.value}`;
    return button.value || "No shortcut";
  }

  function iconMarkup(button, className) {
    const content = button.iconImage ? `<img src="${button.iconImage}" alt="">` : escapeHtml(button.icon || "·");
    return `<span class="${className}">${content}</span>`;
  }

  async function importIcon(event) {
    const file = event.target.files[0]; event.target.value = "";
    if (!file) return;
    if (file.size > 1024 * 1024) return showToast("Icon must be smaller than 1 MB");
    try {
      const image = await loadImage(URL.createObjectURL(file));
      const canvas = document.createElement("canvas"); canvas.width = 96; canvas.height = 96;
      const context = canvas.getContext("2d");
      const scale = Math.min(80 / image.width, 80 / image.height);
      const width = image.width * scale, height = image.height * scale;
      context.drawImage(image, (96 - width) / 2, (96 - height) / 2, width, height);
      updateButton({ iconImage: canvas.toDataURL("image/png") }); showToast("Custom icon added");
    } catch (_) { showToast("That image could not be opened"); }
  }

  async function importBackground(event) {
    const file = event.target.files[0]; event.target.value = "";
    if (!file) return;
    if (file.size > 8 * 1024 * 1024) return showToast("Background must be smaller than 8 MB");
    try {
      const image = await loadImage(URL.createObjectURL(file));
      const canvas = document.createElement("canvas"); canvas.width = 800; canvas.height = 480;
      const context = canvas.getContext("2d"); context.fillStyle = "#071116"; context.fillRect(0, 0, 800, 480);
      const scale = Math.max(800 / image.width, 480 / image.height);
      const width = image.width * scale, height = image.height * scale;
      context.drawImage(image, (800 - width) / 2, (480 - height) / 2, width, height);
      currentProfile().backgroundImage = canvas.toDataURL("image/jpeg", 0.84);
      changed("Background cropped to 800 × 480");
    } catch (_) { showToast("That background could not be opened"); }
  }

  function loadImage(src) {
    return new Promise((resolve, reject) => {
      const image = new Image();
      image.onload = () => { URL.revokeObjectURL(src); resolve(image); };
      image.onerror = () => { URL.revokeObjectURL(src); reject(new Error("image")); };
      image.src = src;
    });
  }

  function exportProject() {
    const payload = clone(project); payload.exportedAt = new Date().toISOString();
    const blob = new Blob([JSON.stringify(payload, null, 2)], { type: "application/json" });
    const link = document.createElement("a"); link.href = URL.createObjectURL(blob);
    link.download = `macrodesk-${project.profiles.map(item => item.id).join("-")}.macrodesk`; link.click();
    setTimeout(() => URL.revokeObjectURL(link.href), 0); showToast("Profile bundle exported");
  }

  async function importProject(event) {
    const file = event.target.files[0]; event.target.value = "";
    if (!file) return;
    try {
      const next = migrateProject(JSON.parse(await file.text())); validateProject(next);
      project = next; project.activeProfile = 0; selection = { area: "main", index: 0 };
      changed("Profile bundle imported");
    } catch (error) { showToast(error.message || "Invalid profile bundle"); }
  }

  function validateProject(value) {
    if (!value || value.format !== "macrodesk-profile" || value.version !== 3) throw new Error("Unsupported MacroDesk file");
    if (!Array.isArray(value.profiles) || value.profiles.length !== 2) throw new Error("A bundle must contain two profiles");
    value.profiles.forEach(profile => {
      const validBackground = profile?.backgroundImage === null || (typeof profile?.backgroundImage === "string" && (
        BUILT_IN_BACKGROUNDS.has(profile.backgroundImage) || /^data:image\/(?:png|jpeg);base64,[A-Za-z0-9+/=]+$/.test(profile.backgroundImage)
      ));
      const expectedMainButtons = profile?.id === "fusion" ? 20 : 24;
      const validPanelLayout = typeof profile?.panelTitle === "string" && profile.panelTitle.length <= 14 && [2, 3].includes(profile.panelColumns) && [2, 3].includes(profile.panelRows) && typeof profile.showQuickAction === "boolean";
      if (!profile || typeof profile.id !== "string" || typeof profile.name !== "string" || typeof profile.badge !== "string" || !validColour(profile.accent) || !validBackground || !validPanelLayout || typeof profile.backgroundDim !== "number" || !Array.isArray(profile.pages) || profile.pages.length !== 8 || !Array.isArray(profile.sidebarButtons) || profile.sidebarButtons.length !== 8 || !Array.isArray(profile.pageButtons) || profile.pageButtons.length !== 8 || profile.pageButtons.some(page => !Array.isArray(page) || page.length !== expectedMainButtons) || !Number.isInteger(profile.activePage) || profile.activePage < 0 || profile.activePage > 7 || !Array.isArray(profile.panelButtons) || profile.panelButtons.length !== 9) throw new Error("Profile does not match the 800 × 480 deck template");
      [...profile.sidebarButtons, ...profile.pageButtons.flat(), ...profile.panelButtons, profile.quickAction].forEach(validateButton);
    });
  }

  function validateButton(button) {
    const validIconImage = button?.iconImage === null || (typeof button?.iconImage === "string" && /^data:image\/png;base64,[A-Za-z0-9+/=]+$/.test(button.iconImage));
    if (!button || typeof button.id !== "string" || typeof button.label !== "string" || !VALID_ACTIONS.has(button.action) || typeof button.value !== "string" || typeof button.icon !== "string" || !validIconImage || !validColour(button.color) || typeof button.enabled !== "boolean") throw new Error("A button contains invalid data");
  }

  function validColour(value) { return typeof value === "string" && /^#[0-9A-Fa-f]{6}$/.test(value); }

  // A version 2 bundle carried one deck per profile; it becomes page 1 here and the
  // remaining seven pages start empty for the user to fill.
  function migrateProject(value) {
    if (!value || value.format !== "macrodesk-profile" || value.version !== 2 || !Array.isArray(value.profiles)) return value;
    value.profiles.forEach(profile => {
      if (!profile || !Array.isArray(profile.mainButtons)) return;
      const count = profile.mainButtons.length;
      profile.pageButtons = [profile.mainButtons].concat(Array.from({ length: 7 },
        (_, page) => makeButtons(profile.id, `page${page + 2}`, [], count, profile.accent)));
      delete profile.mainButtons;
      profile.activePage = 0;
    });
    value.version = 3;
    return value;
  }

  function loadProject() {
    for (const key of [STORAGE_KEY, STORAGE_KEY_V2]) {
      try {
        const value = migrateProject(JSON.parse(localStorage.getItem(key)));
        validateProject(value);
        return value;
      } catch (_) { /* fall through to the older key, then to a fresh project */ }
    }
    return makeProject();
  }

  function showToast(message) {
    elements.toast.textContent = message; elements.toast.classList.add("show");
    clearTimeout(toastTimer); toastTimer = setTimeout(() => elements.toast.classList.remove("show"), 2200);
  }

  function escapeHtml(value) {
    return String(value).replace(/[&<>'"]/g, character => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", "'": "&#39;", '"': "&quot;" })[character]);
  }

  init();
}());
