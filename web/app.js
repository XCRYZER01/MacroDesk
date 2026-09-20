(function () {
  "use strict";

  const STORAGE_KEY = "macrodesk-studio-project-v4";
  const OLD_STORAGE_KEYS = ["macrodesk-studio-project-v3", "macrodesk-studio-project-v2"];
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
  const SIDEBAR_PAGE_KEYS = 12;
  const PAGE_COLUMN_CHOICES = [3, 4, 5];

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
      pages: config.pages.map((name, index) => ({
        name,
        title: index === 0 ? "" : name.toUpperCase().slice(0, 18),
        hint: "",
        columns: 4
      })),
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
        config.id, `page${page + 1}`, page === 0 ? config.main : [], pageKeyCount(config.id, page), config.accent
      )),
      panelButtons: makeButtons(config.id, "panel", config.panel, 9, config.accent),
      quickAction: makeButtons(config.id, "quick", [config.quick], 1, config.accent)[0]
    };
  }

  // Declared, not assigned, because the templates above call them while they are built.
  function mainCount(profileId) { return profileId === "fusion" ? 20 : 24; }
  function pageKeyCount(profileId, page) { return page === 0 ? mainCount(profileId) : SIDEBAR_PAGE_KEYS; }

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
      format: "macrodesk-profile", version: 4,
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
    deviceScreen: $("#deviceScreen"),
    screenGrid: $("#screenGrid"), screenPanelGrid: $("#screenPanelGrid"), screenQuick: $("#screenQuick"),
    screenSidebar: $("#screenSidebar"), screenTabs: $("#screenTabs"), quickAction: $("#quickAction"),
    quickShortcut: $("#quickShortcut"), inspectorFields: $("#inspectorFields"),
    screenPage: $("#screenPage"), screenPageTitle: $("#screenPageTitle"), screenPageHint: $("#screenPageHint"),
    screenPageGrid: $("#screenPageGrid"), pageMeta: $("#pageMeta"), pageTitle: $("#pageTitle"),
    pageHint: $("#pageHint"), pageColumns: $("#pageColumns"),
    selectionPosition: $("#selectionPosition"), buttonLabel: $("#buttonLabel"), buttonAction: $("#buttonAction"),
    buttonValue: $("#buttonValue"), valueLabel: $("#valueLabel"), valueHelp: $("#valueHelp"),
    iconChoices: $("#iconChoices"), buttonColor: $("#buttonColor"), colorValue: $("#colorValue"),
    buttonEnabled: $("#buttonEnabled"), clearButton: $("#clearButton"), importFile: $("#importFile"),
    iconFile: $("#iconFile"), backgroundFile: $("#backgroundFile"), backgroundDim: $("#backgroundDim"),
    backgroundDimValue: $("#backgroundDimValue"), removeBackground: $("#removeBackground"),
    panelTitle: $("#panelTitle"), panelColumns: $("#panelColumns"), panelRows: $("#panelRows"),
    showQuickAction: $("#showQuickAction"),
    saveState: $("#saveState"), toast: $("#toast"), sendToDevice: $("#sendToDevice")
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
    elements.pageTitle.addEventListener("input", event => { currentPage().title = event.target.value; changed(); });
    elements.pageHint.addEventListener("input", event => { currentPage().hint = event.target.value; changed(); });
    elements.pageColumns.addEventListener("change", event => { currentPage().columns = Number(event.target.value); changed(); });
    elements.panelColumns.addEventListener("change", event => { currentProfile().panelColumns = Number(event.target.value); selection = { area: "panel", index: 0 }; changed("Right sidebar layout updated"); });
    elements.panelRows.addEventListener("change", event => { currentProfile().panelRows = Number(event.target.value); selection = { area: "panel", index: 0 }; changed("Right sidebar layout updated"); });
    elements.showQuickAction.addEventListener("change", event => { currentProfile().showQuickAction = event.target.checked; selection = { area: event.target.checked ? "quick" : "panel", index: 0 }; changed("Right sidebar layout updated"); });
    elements.clearButton.addEventListener("click", () => updateButton({ label: "Empty", action: "none", value: "", icon: "−", iconImage: null, enabled: false }));
    $("#resetProfile").addEventListener("click", resetCurrentProfile);
    $("#exportButton").addEventListener("click", exportProject);
    elements.sendToDevice.addEventListener("click", sendProjectToDevice);
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
    return (owner && owner.label) || (profile.pages[index] && profile.pages[index].name) || `Page ${index + 1}`;
  }

  function openPage(index) {
    const profile = currentProfile();
    if (profile.activePage === index && selection.area === "main") return;
    profile.activePage = index;
    selection = { area: "main", index: 0 };
    changed();
  }
  const panelSlotCount = profile => profile.panelColumns * profile.panelRows;
  const currentPage = () => currentProfile().pages[currentProfile().activePage];

  // The board is sent the keys up to the last one in use, so a page with three
  // keys draws three cards -- trailing blanks are never transferred.
  function usedKeyCount(keys) {
    let last = -1;
    keys.forEach((button, index) => { if (button.enabled && button.action !== "none") last = index; });
    return last + 1;
  }
  function isUnmappedButton(button) {
    return !button?.enabled || button?.action === "none" || !String(button?.value || "").trim();
  }
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
    elements.deviceScreen.style.setProperty("--profile-accent", profile.accent);
    elements.deviceScreen.style.setProperty("--main-columns", profile.id === "fusion" ? 5 : 6);
    elements.deviceScreen.style.setProperty("--panel-columns", profile.panelColumns);
    elements.deviceScreen.style.setProperty("--panel-rows", profile.panelRows);
    elements.deviceScreen.classList.toggle("full-right-panel", !profile.showQuickAction);
    elements.panelTitle.value = profile.panelTitle;
    const page = profile.pages[profile.activePage];
    elements.pageMeta.hidden = profile.activePage === 0;
    elements.pageTitle.value = page.title;
    elements.pageHint.value = page.hint;
    elements.pageColumns.value = String(page.columns);
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
    renderMainArea(profile);
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

  // Page 1 is the artwork page. Every other page is the LVGL overlay, laid out
  // with the same arithmetic build_page() uses on the device.
  function renderMainArea(profile) {
    const keys = profile.pageButtons[profile.activePage];
    const onImagePage = profile.activePage === 0;
    elements.screenGrid.hidden = !onImagePage;
    elements.screenPage.hidden = onImagePage;
    if (onImagePage) {
      renderPreviewButtons(elements.screenGrid, keys, "main", "screen-key");
      return;
    }
    const page = profile.pages[profile.activePage];
    const count = usedKeyCount(keys);
    const columns = page.columns;
    const rows = Math.max(1, Math.ceil(count / columns));
    const height = Math.min(100, (288 - (rows - 1) * 8) / rows);
    elements.screenPageTitle.textContent = page.title || pageName(profile, profile.activePage);
    elements.screenPageHint.textContent = page.hint;
    elements.screenPage.style.setProperty("--page-columns", columns);
    elements.screenPage.style.setProperty("--page-row-height", `${(height / 800 * 100).toFixed(4)}cqw`);
    if (!count) {
      elements.screenPageGrid.innerHTML = '<span class="screen-page-empty">No keys yet — fill a slot in the list on the left.</span>';
      return;
    }
    renderPreviewButtons(elements.screenPageGrid, keys.slice(0, count), "main", "screen-page-key");
  }

  function renderPreviewButtons(container, buttons, area, className) {
    container.innerHTML = "";
    buttons.forEach((button, index) => {
      const key = document.createElement("button");
      key.type = "button";
      const empty = area === "panel" && isUnmappedButton(button);
      key.className = `${className}${selection.area === area && selection.index === index ? " selected" : ""}${button.enabled ? "" : " disabled"}${empty ? " empty" : ""}`;
      if (area === "panel") key.style.order = empty ? "1" : "0";
      key.style.setProperty("--button-accent", button.color);
      if (!empty) key.innerHTML = `${iconMarkup(button, "key-icon")}<strong>${escapeHtml(button.label || "Untitled")}</strong><small>${escapeHtml(actionSummary(button))}</small>`;
      key.setAttribute("aria-label", empty ? `Empty right sidebar slot ${index + 1}` : (button.label || `Key ${index + 1}`));
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

  class BundleWriter {
    constructor() { this.parts = []; this.length = 0; }
    append(bytes) { this.parts.push(bytes); this.length += bytes.length; }
    u8(value) { this.append(Uint8Array.of(value & 0xFF)); }
    u16(value) { this.append(Uint8Array.of(value & 0xFF, (value >>> 8) & 0xFF)); }
    i16(value) { this.u16(value & 0xFFFF); }
    u32(value) { this.append(Uint8Array.of(value & 0xFF, (value >>> 8) & 0xFF, (value >>> 16) & 0xFF, (value >>> 24) & 0xFF)); }
    string(value) {
      const bytes = new TextEncoder().encode(String(value || ""));
      if (bytes.length > 65535) throw new Error("Text is too long for the device");
      this.u16(bytes.length); this.append(bytes); this.u8(0);
    }
    align4() { while (this.length % 4) this.u8(0); }
    finish() {
      const output = new Uint8Array(this.length); let offset = 0;
      this.parts.forEach(part => { output.set(part, offset); offset += part.length; });
      return output;
    }
  }

  function crc32(bytes) {
    let crc = 0xFFFFFFFF;
    for (const value of bytes) {
      crc ^= value;
      for (let bit = 0; bit < 8; bit += 1) crc = (crc >>> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
    }
    return (crc ^ 0xFFFFFFFF) >>> 0;
  }

  function hexColour(value) { return Number.parseInt(String(value || "#000000").slice(1), 16) >>> 0; }
  function darkerColour(value) {
    const raw = hexColour(value);
    return (((Math.floor(((raw >>> 16) & 0xFF) * .62)) << 16)
      | ((Math.floor(((raw >>> 8) & 0xFF) * .62)) << 8)
      | Math.floor((raw & 0xFF) * .62)) >>> 0;
  }

  function runtimeButton(button, override = null) {
    let action = 0, arg = 0, keys = button.value || "";
    if (override) ({ action, arg, keys = "" } = override);
    else if (!button.enabled || button.action === "none" || !button.value) {
      action = 1; keys = "";
    } else if (button.action === "page") {
      action = 3; arg = Math.max(0, Math.min(7, Number.parseInt(button.value, 10) - 1 || 0)); keys = "";
    } else if (button.action === "search") keys = `search:${button.value}`;
    else if (button.action === "text") action = 8;
    const rawIcon = String(button.icon || "");
    const fallbackIcon = (String(button.label || "?").trim()[0] || "?").toUpperCase();
    const icon = /^[\x20-\x7E]{1,4}$/.test(rawIcon) ? rawIcon : (/^[\x20-\x7E]$/.test(fallbackIcon) ? fallbackIcon : "?");
    return { action, arg, accent: hexColour(button.color), icon, label: button.label || "", keys };
  }

  function writeRuntimeButton(writer, button) {
    writer.u8(button.action); writer.u8(button.arg); writer.u32(button.accent);
    writer.string(button.icon); writer.string(button.label); writer.string(button.keys);
  }

  function runtimeZones(profile, profileIndex, profiles) {
    const zones = [];
    const add = (x, y, w, h, button) => zones.push({ x, y, w, h, button: runtimeButton(button) });
    profile.sidebarButtons.slice(0, 8).forEach((button, row) => add(4, 84 + row * 42, 123, 41, button));

    const main = profile.pageButtons[0], columns = main.length === 20 ? 5 : 6;
    const [left, pitch, width] = columns === 5 ? [133, 97, 93] : [134, 80, 76];
    main.forEach((button, slot) => add(left + (slot % columns) * pitch, 88 + Math.floor(slot / columns) * 85, width, 82, button));

    if (profile.showQuickAction) add(620, 88, 168, 63, profile.quickAction);
    const panelColumns = profile.panelColumns, panelRows = profile.panelRows;
    const panelCount = panelColumns * panelRows;
    const panelLeft = 626, panelWidth = 160, gapX = 2;
    const cellWidth = Math.floor((panelWidth - gapX * (panelColumns - 1)) / panelColumns);
    const panelTop = profile.showQuickAction ? 186 : 128;
    const panelHeight = profile.showQuickAction ? 204 : 264, gapY = 6;
    const cellHeight = Math.floor((panelHeight - gapY * (panelRows - 1)) / panelRows);
    const panelButtons = profile.panelButtons.slice(0, panelCount);
    const orderedPanelButtons = panelButtons.filter(button => !isUnmappedButton(button))
      .concat(panelButtons.filter(isUnmappedButton));
    orderedPanelButtons.forEach((button, slot) => {
      const column = slot % panelColumns, row = Math.floor(slot / panelColumns);
      // Keep every right-sidebar zone in the bundle so its slot still exists.
      // Mapped keys pack from the top-left. Unmapped zones move to the end and
      // have an empty visual label, so firmware retains their geometry without
      // drawing a card over the artwork.
      const runtimePanelButton = isUnmappedButton(button) ? { ...button, label: "", icon: "" } : button;
      add(panelLeft + column * (cellWidth + gapX), panelTop + row * (cellHeight + gapY), cellWidth, cellHeight, runtimePanelButton);
    });

    const tab = (index, x, width) => ({
      x, y: 432, w: width, h: 40,
      button: runtimeButton({ label: profiles[index].name, icon: profiles[index].badge, color: profiles[index].accent }, { action: 2, arg: index })
    });
    zones.push(tab(0, 78, 128), tab(1, 208, 128), {
      x: 337, y: 432, w: 118, h: 40,
      button: runtimeButton({ label: "System", icon: "S", color: profile.accent }, { action: 1, arg: 0 })
    });
    return zones;
  }

  function usedPageButtons(buttons) {
    let last = -1;
    buttons.forEach((button, index) => { if (button.enabled && button.action !== "none") last = index; });
    return buttons.slice(0, last + 1);
  }

  async function renderRuntimeImage(profile) {
    if (location.protocol === "file:" && profile.backgroundImage && !profile.backgroundImage.startsWith("data:")) {
      throw new Error("Open Studio from http://localhost before sending built-in backgrounds");
    }
    const canvas = document.createElement("canvas"); canvas.width = 800; canvas.height = 480;
    const context = canvas.getContext("2d", { willReadFrequently: true });
    context.fillStyle = "#071116"; context.fillRect(0, 0, 800, 480);
    if (profile.backgroundImage) {
      const image = await new Promise((resolve, reject) => {
        const next = new Image();
        if (!profile.backgroundImage.startsWith("data:")) next.crossOrigin = "anonymous";
        next.onload = () => resolve(next);
        next.onerror = () => reject(new Error(`Could not load ${profile.name} background; serve the repository from localhost`));
        next.src = profile.backgroundImage;
      });
      context.drawImage(image, 0, 0, 800, 480);
      const dim = Math.max(0, Math.min(1, Number(profile.backgroundDim) || 0));
      if (dim) { context.fillStyle = `rgba(7,17,22,${dim})`; context.fillRect(0, 0, 800, 480); }
    }
    const rgba = context.getImageData(0, 0, 800, 480).data;
    const rgb565 = new Uint8Array(800 * 480 * 2);
    for (let pixel = 0, source = 0, target = 0; pixel < 800 * 480; pixel += 1, source += 4, target += 2) {
      const value = ((rgba[source] & 0xF8) << 8) | ((rgba[source + 1] & 0xFC) << 3) | (rgba[source + 2] >>> 3);
      rgb565[target] = value & 0xFF; rgb565[target + 1] = value >>> 8;
    }
    return rgb565;
  }

  async function buildRuntimeBundle(value) {
    validateProject(value);
    const writer = new BundleWriter();
    for (let profileIndex = 0; profileIndex < value.profiles.length; profileIndex += 1) {
      const profile = value.profiles[profileIndex];
      writer.string(profile.name); writer.u32(darkerColour(profile.accent)); writer.u32(hexColour(profile.accent));
      writer.align4();
      const image = await renderRuntimeImage(profile);
      writer.u32(image.length); writer.append(image);

      const zones = runtimeZones(profile, profileIndex, value.profiles);
      writer.u16(zones.length);
      zones.forEach(zone => {
        writer.i16(zone.x); writer.i16(zone.y); writer.i16(zone.w); writer.i16(zone.h); writeRuntimeButton(writer, zone.button);
      });

      writer.u8(profile.pages.length);
      profile.pages.forEach((page, pageIndex) => {
        const buttons = pageIndex === 0 ? [] : usedPageButtons(profile.pageButtons[pageIndex]).map(button => runtimeButton(button));
        writer.string(page.title); writer.string(page.hint); writer.u8(pageIndex === 0 ? 0 : page.columns); writer.u8(buttons.length);
        buttons.forEach(button => writeRuntimeButton(writer, button));
      });
    }
    const payload = writer.finish();
    const bundle = new Uint8Array(16 + payload.length); bundle.set([0x4D, 0x44, 0x42, 0x31], 0);
    const header = new DataView(bundle.buffer); header.setUint16(4, 1, true); bundle[6] = value.profiles.length;
    header.setUint32(8, bundle.length, true); header.setUint32(12, crc32(payload), true); bundle.set(payload, 16);
    return bundle;
  }

  async function waitForProtocol(reader, state, accepted, timeoutMs) {
    const deadline = Date.now() + timeoutMs;
    while (Date.now() < deadline) {
      const remaining = deadline - Date.now();
      const result = await Promise.race([
        reader.read(),
        new Promise((_, reject) => setTimeout(() => reject(new Error("Device response timed out")), remaining))
      ]);
      if (result.done) throw new Error("Device disconnected");
      state.text += state.decoder.decode(result.value, { stream: true });
      const lines = state.text.split(/\r?\n/); state.text = lines.pop();
      for (const line of lines) {
        if (line.startsWith("MDERR ")) throw new Error(`Device rejected profile: ${line.slice(6)}`);
        if (accepted.some(prefix => line.startsWith(prefix))) return line;
      }
    }
    throw new Error("Device response timed out");
  }

  async function sendProjectToDevice() {
    if (!("serial" in navigator)) return showToast("Use Chrome or Edge: this browser has no Web Serial");
    if (location.protocol === "file:") return showToast("Serve the repository, then open Studio at http://localhost");
    const button = elements.sendToDevice, original = button.textContent;
    let port, reader, writer;
    button.disabled = true;
    try {
      port = await navigator.serial.requestPort({ filters: [{ usbVendorId: 0x1A86, usbProductId: 0x55D3 }] });
      button.textContent = "Preparing…"; showToast("Preparing images for the device…");
      const snapshot = clone(project);
      const bitmapIconCount = snapshot.profiles.reduce((total, profile) => total +
        [...profile.sidebarButtons, ...profile.pageButtons.flat(), ...profile.panelButtons, profile.quickAction]
          .filter(item => item.iconImage).length, 0);
      const bundle = await buildRuntimeBundle(snapshot);
      await port.open({ baudRate: 115200, bufferSize: 65536 });
      reader = port.readable.getReader(); writer = port.writable.getWriter();
      const state = { decoder: new TextDecoder(), text: "" };
      button.textContent = "Waking device…";
      // Opening CH343 toggles the ESP32-S3 reset lines on some Windows drivers.
      // Let the full display/UI boot finish before sending the upload command;
      // bytes written during that reset window can disappear without an error.
      await new Promise(resolve => setTimeout(resolve, 6000));
      const bundleCrc = crc32(bundle).toString(16).padStart(8, "0");
      button.textContent = "Starting transfer…";
      await writer.write(new TextEncoder().encode(`MDUP ${bundle.length} ${bundleCrc}\n`));
      try {
        await waitForProtocol(reader, state, ["MDREADY "], 30000);
      } catch (error) {
        if (error.message === "Device response timed out") throw new Error("Device did not answer MDREADY after restart");
        throw error;
      }
      const chunkSize = 512;
      const progressWindow = 4 * 1024;
      for (let windowStart = 0; windowStart < bundle.length; windowStart += progressWindow) {
        const windowEnd = Math.min(bundle.length, windowStart + progressWindow);
        for (let offset = windowStart; offset < windowEnd; offset += chunkSize) {
          await writer.write(bundle.subarray(offset, Math.min(windowEnd, offset + chunkSize)));
        }
        if (windowEnd < bundle.length) {
          try {
            await waitForProtocol(reader, state, ["MDPROGRESS "], 15000);
          } catch (error) {
            if (error.message === "Device response timed out") {
              throw new Error(`Device stopped receiving at ${Math.round(windowStart * 100 / bundle.length)}%`);
            }
            throw error;
          }
          button.textContent = `Sending ${Math.round(windowEnd * 100 / bundle.length)}%`;
        }
      }
      button.textContent = "Verifying…";
      try {
        await waitForProtocol(reader, state, ["MDOK "], 30000);
      } catch (error) {
        if (error.message === "Device response timed out") throw new Error("Device did not finish verifying the profile");
        throw error;
      }
      showToast(bitmapIconCount
        ? `Profile installed; ${bitmapIconCount} bitmap icon(s) use text fallback on the board`
        : "Profile installed — the board is restarting");
    } catch (error) {
      showToast(error.message || "Could not send profile to the device");
    } finally {
      if (reader) { try { await reader.cancel(); } catch (_) {} reader.releaseLock(); }
      if (writer) { try { await writer.close(); } catch (_) {} writer.releaseLock(); }
      if (port) { try { await port.close(); } catch (_) {} }
      button.disabled = false; button.textContent = original;
    }
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
      const next = migrateV3(migrateProject(JSON.parse(await file.text()))); validateProject(next);
      project = next; project.activeProfile = 0; selection = { area: "main", index: 0 };
      changed("Profile bundle imported");
    } catch (error) { showToast(error.message || "Invalid profile bundle"); }
  }

  function validateProject(value) {
    if (!value || value.format !== "macrodesk-profile" || value.version !== 4) throw new Error("Unsupported MacroDesk file");
    if (!Array.isArray(value.profiles) || value.profiles.length !== 2) throw new Error("A bundle must contain two profiles");
    value.profiles.forEach(profile => {
      const validBackground = profile?.backgroundImage === null || (typeof profile?.backgroundImage === "string" && (
        BUILT_IN_BACKGROUNDS.has(profile.backgroundImage) || /^data:image\/(?:png|jpeg);base64,[A-Za-z0-9+/=]+$/.test(profile.backgroundImage)
      ));
      const expectedMainButtons = profile?.id === "fusion" ? 20 : 24;
      const validPage = page => page && typeof page.name === "string" && page.name.length <= 18
        && typeof page.title === "string" && page.title.length <= 18
        && typeof page.hint === "string" && page.hint.length <= 64
        && PAGE_COLUMN_CHOICES.includes(page.columns);
      const validPanelLayout = typeof profile?.panelTitle === "string" && profile.panelTitle.length <= 14 && [2, 3].includes(profile.panelColumns) && [2, 3].includes(profile.panelRows) && typeof profile.showQuickAction === "boolean";
      if (!profile || typeof profile.id !== "string" || typeof profile.name !== "string" || typeof profile.badge !== "string" || !validColour(profile.accent) || !validBackground || !validPanelLayout || typeof profile.backgroundDim !== "number" || !Array.isArray(profile.pages) || profile.pages.length !== 8 || !Array.isArray(profile.sidebarButtons) || profile.sidebarButtons.length !== 8 || !Array.isArray(profile.pageButtons) || profile.pageButtons.length !== 8 || profile.pageButtons.some((page, index) => !Array.isArray(page) || page.length !== (index === 0 ? expectedMainButtons : SIDEBAR_PAGE_KEYS)) || !profile.pages.every(validPage) || !Number.isInteger(profile.activePage) || profile.activePage < 0 || profile.activePage > 7 || !Array.isArray(profile.panelButtons) || profile.panelButtons.length !== 9) throw new Error("Profile does not match the 800 × 480 deck template");
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
    return migrateV3(value);
  }

  // v3 sized every page like the artwork page and had no page titles. Sub-pages
  // are cut to what the board's LVGL heap can actually draw.
  function migrateV3(value) {
    if (!value || value.format !== "macrodesk-profile" || value.version !== 3 || !Array.isArray(value.profiles)) return value;
    value.profiles.forEach(profile => {
      if (!profile || !Array.isArray(profile.pageButtons)) return;
      profile.pages = (profile.pages || []).map((page, index) => typeof page === "string"
        ? { name: page, title: index === 0 ? "" : page.toUpperCase().slice(0, 18), hint: "", columns: 4 }
        : page);
      profile.pageButtons = profile.pageButtons.map((keys, index) => {
        if (index === 0) return keys;
        const trimmed = keys.slice(0, SIDEBAR_PAGE_KEYS);
        while (trimmed.length < SIDEBAR_PAGE_KEYS) {
          trimmed.push(makeButtons(profile.id, `page${index + 1}`, [], SIDEBAR_PAGE_KEYS, profile.accent)[trimmed.length]);
        }
        return trimmed;
      });
    });
    value.version = 4;
    return value;
  }

  function loadProject() {
    for (const key of [STORAGE_KEY, ...OLD_STORAGE_KEYS]) {
      try {
        const value = migrateV3(migrateProject(JSON.parse(localStorage.getItem(key))));
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
