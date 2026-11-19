"use strict";

class BinaryDiagram extends HTMLElement {
  static rows = 32;
  static active = null;
  static tooltip = null;
  static helpOwner = null;
  static helpTooltip = null;
  static filterOwner = null;
  static filterMenu = null;

  static node(tag, className, text) {
    const result = document.createElement(tag);
    result.className = className;
    if (text !== undefined) result.textContent = text;
    return result;
  }

  static categoryLabel(category) {
    return category.id === "empty" ? "Padding" : category.label;
  }

  static isAuxiliaryTarget(target) {
    return target?.closest?.(".binary-help-button, .binary-filter-button")
      || this.helpTooltip?.contains(target) || this.filterMenu?.contains(target);
  }

  static sharedTooltip() {
    if (this.tooltip) return this.tooltip;
    const tooltip = document.createElement("div");
    tooltip.className = "binary-tooltip";
    tooltip.id = "binary-span-tooltip";
    tooltip.setAttribute("role", "group");
    tooltip.setAttribute("aria-label", "Binary span details");
    tooltip.setAttribute("aria-live", "polite");
    tooltip.hidden = true;
    tooltip.addEventListener("pointerdown", (event) => event.preventDefault());
    tooltip.addEventListener("click", (event) => {
      const active = this.active;
      if (!active || event.target.closest(".binary-source-link")) return;
      active.pinned = true;
      active.canvas.focus({ preventScroll: true });
    });
    tooltip.addEventListener("pointerleave", (event) => {
      const active = this.active;
      if (active && !active.pinned && event.relatedTarget !== active.canvas) active.clear();
    });
    document.body.append(tooltip);
    this.tooltip = tooltip;
    const dismissOutside = (event) => {
      const active = this.active;
      if (!active || event.target === active.canvas || active.navigation.contains(event.target) || tooltip.contains(event.target) || this.isAuxiliaryTarget(event.target)) return;
      active.clear();
      active.navigation.hidden = true;
      if (active.contains(document.activeElement)) document.activeElement.blur();
    };
    document.addEventListener("pointerdown", dismissOutside, true);
    document.addEventListener("click", dismissOutside, true);
    document.addEventListener("focusin", (event) => {
      const active = this.active;
      if (!active || active.contains(event.target) || this.isAuxiliaryTarget(event.target)) return;
      active.navigation.hidden = true;
      active.clear();
    });
    return tooltip;
  }

  static sharedHelp() {
    if (this.helpTooltip) return this.helpTooltip;
    const tooltip = this.node("div", "binary-help-tooltip");
    tooltip.id = "binary-help-tooltip";
    tooltip.setAttribute("role", "tooltip");
    tooltip.hidden = true;
    tooltip.addEventListener("pointerdown", (event) => event.preventDefault());
    document.body.append(tooltip);
    this.helpTooltip = tooltip;
    const dismissOutside = (event) => {
      const owner = this.helpOwner;
      if (!owner || owner.helpButton.contains(event.target) || tooltip.contains(event.target)) return;
      owner.closeHelp();
    };
    document.addEventListener("pointerdown", dismissOutside, true);
    document.addEventListener("click", dismissOutside, true);
    window.addEventListener("scroll", (event) => {
      const owner = this.helpOwner;
      if (!owner) return;
      if ((event.target === document || event.target === window)
        && owner.helpScrollX === window.scrollX && owner.helpScrollY === window.scrollY) return;
      owner.closeHelp();
    }, true);
    window.addEventListener("resize", () => this.helpOwner?.positionHelp());
    return tooltip;
  }

  static sharedFilter() {
    if (this.filterMenu) return this.filterMenu;
    const menu = this.node("div", "binary-filter-menu");
    menu.id = "binary-filter-menu";
    menu.setAttribute("role", "dialog");
    menu.setAttribute("aria-label", "Visible classifications");
    menu.hidden = true;
    menu.addEventListener("keydown", (event) => {
      if (event.key !== "Escape") return;
      event.preventDefault();
      this.filterOwner?.closeFilter();
    });
    document.body.append(menu);
    this.filterMenu = menu;
    const dismissOutside = (event) => {
      const owner = this.filterOwner;
      if (!owner || owner.filterButton.contains(event.target) || menu.contains(event.target)) return;
      owner.closeFilter();
    };
    document.addEventListener("pointerdown", dismissOutside, true);
    document.addEventListener("click", dismissOutside, true);
    document.addEventListener("focusin", dismissOutside);
    window.addEventListener("scroll", (event) => {
      const owner = this.filterOwner;
      if (!owner || menu.contains(event.target)) return;
      if ((event.target === document || event.target === window)
        && owner.filterScrollX === window.scrollX && owner.filterScrollY === window.scrollY) return;
      owner.closeFilter();
    }, true);
    window.addEventListener("resize", () => this.filterOwner?.positionFilter());
    return menu;
  }

  configure(binary, categories, target) {
    this.binary = binary;
    this.categories = categories;
    this.target = target;
    this.hiddenCategories = new Set();
  }

  connectedCallback() {
    if (!this.binary) return;
    this.selected = -1;
    this.pinned = false;
    this.events = new AbortController();
    const node = BinaryDiagram.node;
    this.surface = node("div", "binary-surface");
    this.canvas = node("canvas", "binary-canvas");
    this.canvas.tabIndex = 0;
    this.canvas.setAttribute("role", "img");
    this.canvas.setAttribute("aria-label", `${this.target}: original binary byte map. Arrow keys select spans; Home and End select bounds; Escape clears selection.`);
    BinaryDiagram.sharedTooltip();
    BinaryDiagram.sharedHelp();
    BinaryDiagram.sharedFilter();
    const heading = node("div", "binary-heading");
    this.helpButton = node("button", "binary-help-button", "?");
    this.helpButton.type = "button";
    this.helpButton.setAttribute("aria-label", "Binary classification legend");
    this.helpButton.setAttribute("aria-controls", BinaryDiagram.helpTooltip.id);
    this.helpButton.setAttribute("aria-expanded", "false");
    this.helpButton.addEventListener("click", () => {
      if (BinaryDiagram.helpOwner === this) this.closeHelp();
      else this.showHelp();
    });
    this.helpButton.addEventListener("keydown", (event) => {
      if (event.key !== "Escape") return;
      event.preventDefault();
      this.closeHelp();
    });
    this.filterButton = node("button", "binary-filter-button");
    this.filterButton.type = "button";
    this.filterButton.setAttribute("aria-label", "Filter classifications");
    this.filterButton.setAttribute("aria-haspopup", "dialog");
    this.filterButton.setAttribute("aria-controls", BinaryDiagram.filterMenu.id);
    this.filterButton.setAttribute("aria-expanded", "false");
    const icon = document.createElementNS("http://www.w3.org/2000/svg", "svg");
    icon.setAttribute("viewBox", "0 0 24 24");
    icon.setAttribute("aria-hidden", "true");
    const path = document.createElementNS(icon.namespaceURI, "path");
    path.setAttribute("d", "M4 5h16l-6 7v6l-4 2v-8z");
    icon.append(path);
    this.filterButton.append(icon);
    this.filterButton.addEventListener("click", () => {
      if (BinaryDiagram.filterOwner === this) this.closeFilter();
      else this.showFilter();
    });
    this.filterButton.addEventListener("keydown", (event) => {
      if (event.key !== "Escape") return;
      event.preventDefault();
      this.closeFilter();
    });
    this.navigation = node("div", "binary-navigation");
    this.navigation.hidden = true;
    this.previous = node("button", "", "‹");
    this.next = node("button", "", "›");
    for (const [button, label, step] of [[this.previous, "Previous byte span", -1], [this.next, "Next byte span", 1]]) {
      button.type = "button";
      button.setAttribute("aria-label", label);
      button.addEventListener("click", () => this.navigate(step));
      this.navigation.append(button);
    }
    const controls = node("div", "binary-controls");
    controls.append(this.navigation, this.filterButton, this.helpButton);
    heading.append(node("p", "metric-label", "Binary layout"), controls);
    this.surface.append(this.canvas);
    const bounds = node("p", "binary-bounds");
    bounds.append(node("span", "", "0x000000"), node("span", "", `${this.hex(this.binary.size - 1)} · ${this.binary.size.toLocaleString("en-US")}b`));
    this.replaceChildren(heading, this.surface, bounds);
    this.updateNavigation();
    this.canvas.addEventListener("pointermove", (event) => {
      if (!this.pinned && !BinaryDiagram.active?.pinned) this.select(this.hit(event), event);
    });
    this.canvas.addEventListener("pointerleave", (event) => {
      if (!this.pinned && !BinaryDiagram.tooltip.contains(event.relatedTarget)) this.clear();
    });
    this.canvas.addEventListener("mousedown", (event) => event.preventDefault());
    this.canvas.addEventListener("click", (event) => {
      this.canvas.focus({ preventScroll: true });
      this.pinned = true;
      this.select(this.hit(event), event);
    });
    this.addEventListener("keydown", (event) => {
      if (this.isSelectionTarget(event.target)) this.key(event);
    }, { signal: this.events.signal });
    this.addEventListener("focusin", (event) => {
      if (!this.isSelectionTarget(event.target) || this.isSelectionTarget(event.relatedTarget)) return;
      this.navigation.hidden = false;
      this.pinned = true;
      if (BinaryDiagram.active === this && this.selected >= 0) this.draw();
      else this.select(this.findVisible(-1, 1));
    }, { signal: this.events.signal });
    this.addEventListener("focusout", (event) => {
      if (this.contains(event.relatedTarget)) return;
      this.navigation.hidden = true;
      if (BinaryDiagram.isAuxiliaryTarget(event.relatedTarget)) this.draw();
      else this.clear();
    }, { signal: this.events.signal });
    this.resize = new ResizeObserver(() => this.draw());
    this.resize.observe(this.canvas);
    this.visibility = new IntersectionObserver(([entry]) => {
      this.visible = entry.isIntersecting;
      if (this.visible) this.draw();
    });
    this.visibility.observe(this);
    this.watchDensity();
  }

  disconnectedCallback() {
    this.closeFilter();
    this.closeHelp();
    if (this.canvas) this.clear();
    this.resize?.disconnect();
    this.visibility?.disconnect();
    this.events?.abort();
  }

  showHelp() {
    BinaryDiagram.filterOwner?.closeFilter();
    BinaryDiagram.helpOwner?.closeHelp();
    const node = BinaryDiagram.node;
    const legend = node("ul", "binary-legend");
    legend.setAttribute("aria-label", "Binary classification legend");
    for (const category of this.categories) {
      if (this.hiddenCategories.has(category.id)) continue;
      const item = node("li", "");
      const chip = node("i", "binary-chip");
      chip.style.backgroundColor = category.color;
      chip.setAttribute("aria-hidden", "true");
      item.append(chip, BinaryDiagram.categoryLabel(category));
      legend.append(item);
    }
    const tooltip = BinaryDiagram.helpTooltip;
    tooltip.replaceChildren(node("p", "metric-label", "Binary legend"),
      legend.childElementCount ? legend : node("div", "", "No classifications visible."));
    this.append(tooltip);
    BinaryDiagram.helpOwner = this;
    this.helpButton.setAttribute("aria-expanded", "true");
    this.helpButton.setAttribute("aria-describedby", tooltip.id);
    this.helpScrollX = window.scrollX;
    this.helpScrollY = window.scrollY;
    tooltip.hidden = false;
    this.positionHelp();
  }

  closeHelp() {
    if (BinaryDiagram.helpOwner !== this) return;
    BinaryDiagram.helpTooltip.hidden = true;
    document.body.append(BinaryDiagram.helpTooltip);
    BinaryDiagram.helpOwner = null;
    this.helpButton.setAttribute("aria-expanded", "false");
    this.helpButton.removeAttribute("aria-describedby");
  }

  positionHelp() {
    this.positionPanel(BinaryDiagram.helpTooltip, this.helpButton);
  }

  showFilter() {
    BinaryDiagram.helpOwner?.closeHelp();
    BinaryDiagram.filterOwner?.closeFilter();
    const node = BinaryDiagram.node;
    const options = node("div", "binary-filters");
    for (const category of this.categories) {
      const label = node("label", "");
      const checkbox = document.createElement("input");
      checkbox.type = "checkbox";
      checkbox.checked = !this.hiddenCategories.has(category.id);
      checkbox.addEventListener("change", () => this.setCategoryVisible(category.id, checkbox.checked));
      const chip = node("i", "binary-chip");
      chip.style.backgroundColor = category.color;
      chip.setAttribute("aria-hidden", "true");
      label.append(checkbox, chip, BinaryDiagram.categoryLabel(category));
      options.append(label);
    }
    const menu = BinaryDiagram.filterMenu;
    menu.replaceChildren(node("p", "metric-label", "Visible classifications"), options);
    this.append(menu);
    BinaryDiagram.filterOwner = this;
    this.filterButton.setAttribute("aria-expanded", "true");
    this.filterScrollX = window.scrollX;
    this.filterScrollY = window.scrollY;
    menu.hidden = false;
    this.positionFilter();
    menu.querySelector("input").focus({ preventScroll: true });
  }

  closeFilter() {
    if (BinaryDiagram.filterOwner !== this) return;
    const menu = BinaryDiagram.filterMenu;
    BinaryDiagram.filterOwner = null;
    if (menu.contains(document.activeElement)) this.filterButton.focus({ preventScroll: true });
    menu.hidden = true;
    document.body.append(menu);
    this.filterButton.setAttribute("aria-expanded", "false");
  }

  positionFilter() {
    this.positionPanel(BinaryDiagram.filterMenu, this.filterButton);
  }

  positionPanel(panel, anchor) {
    const button = anchor.getBoundingClientRect();
    const canvas = this.canvas.getBoundingClientRect();
    panel.style.width = `${Math.min(280, canvas.width, window.innerWidth - 16)}px`;
    const height = panel.offsetHeight;
    const top = button.top >= height + 8 ? button.top - height - 8 : canvas.bottom + 8;
    panel.style.top = `${Math.max(8, Math.min(top, window.innerHeight - height - 8))}px`;
    panel.style.left = `${Math.max(8, Math.min(canvas.right - panel.offsetWidth, window.innerWidth - panel.offsetWidth - 8))}px`;
  }

  watchDensity() {
    window.matchMedia(`(resolution: ${window.devicePixelRatio}dppx)`).addEventListener("change", () => {
      this.draw();
      this.watchDensity();
    }, { once: true, signal: this.events.signal });
  }

  hex(value) {
    return `0x${value.toString(16).toUpperCase().padStart(6, "0")}`;
  }

  hit(event) {
    const rect = this.canvas.getBoundingClientRect();
    const pointX = Math.max(0, Math.min(rect.width, event.clientX - rect.left));
    const pointY = Math.max(0, Math.min(rect.height, event.clientY - rect.top));
    const row = Math.min(BinaryDiagram.rows - 1, Math.floor(pointY / rect.height * BinaryDiagram.rows));
    const x = pointX / rect.width;
    const offset = Math.min(this.binary.size - 1, Math.floor((row + x) / BinaryDiagram.rows * this.binary.size));
    let low = 0;
    let high = this.binary.spans.length - 1;
    while (low < high) {
      const middle = Math.floor((low + high) / 2);
      if (this.binary.spans[middle].end <= offset) low = middle + 1;
      else high = middle;
    }
    if (this.isVisible(low)) return low;
    let nearest = -1;
    let distance = Infinity;
    for (let index = 0; index < this.binary.spans.length; index++) {
      if (!this.isVisible(index)) continue;
      this.rectangles(this.binary.spans[index], rect.width, rect.height, (left, top, width, height) => {
        const dx = Math.max(left - pointX, 0, pointX - left - width);
        const dy = Math.max(top - pointY, 0, pointY - top - height);
        const squared = dx * dx + dy * dy;
        if (squared < distance) {
          nearest = index;
          distance = squared;
        }
      });
    }
    return nearest;
  }

  isVisible(index) {
    const span = this.binary.spans[index];
    return span !== undefined && !this.hiddenCategories.has(span.category);
  }

  findVisible(index, step) {
    for (let next = index + step; next >= 0 && next < this.binary.spans.length; next += step) {
      if (this.isVisible(next)) return next;
    }
    return -1;
  }

  updateNavigation() {
    this.previous.setAttribute("aria-disabled", this.findVisible(this.selected, -1) < 0);
    this.next.setAttribute("aria-disabled", this.findVisible(this.selected, 1) < 0);
  }

  setCategoryVisible(category, visible) {
    if (visible) this.hiddenCategories.delete(category);
    else this.hiddenCategories.add(category);
    this.filterButton.dataset.filtered = String(this.hiddenCategories.size > 0);
    if (this.selected >= 0 && !this.isVisible(this.selected)) {
      const next = this.findVisible(this.selected, 1);
      this.select(next >= 0 ? next : this.findVisible(this.selected, -1));
    } else {
      this.draw();
    }
    this.updateNavigation();
  }

  isSelectionTarget(target) {
    return target === this.canvas || this.navigation.contains(target)
      || (BinaryDiagram.active === this && BinaryDiagram.tooltip.contains(target));
  }

  select(index, event) {
    if (!this.isVisible(index)) {
      this.clear();
      return;
    }
    if (BinaryDiagram.active !== this) {
      BinaryDiagram.active?.clear();
      BinaryDiagram.active = this;
      this.surface.append(BinaryDiagram.tooltip);
    }
    this.selected = index;
    const span = this.binary.spans[index];
    const category = this.categories.find((row) => row.id === span.category);
    const bounds = `${this.hex(span.start)}–${this.hex(span.end - 1)} · ${(span.end - span.start).toLocaleString("en-US")}b`;
    const tooltip = BinaryDiagram.tooltip;
    const summary = BinaryDiagram.node("div", "binary-summary");
    summary.append(
      BinaryDiagram.node("span", "binary-category", span.functions ? "Code" : BinaryDiagram.categoryLabel(category)),
      BinaryDiagram.node("small", "binary-range", bounds),
    );
    tooltip.replaceChildren(summary);
    if (span.functions) {
      for (const fn of span.functions) {
        const row = BinaryDiagram.node("div", "binary-function");
        const label = { exact: "Exact", partial: "Partial", unlifted: "Not started", invalid: "Invalid lift" }[fn.status];
        const status = BinaryDiagram.node("span", "binary-function-status", { exact: "✓", partial: "⏳", unlifted: "×", invalid: "×" }[fn.status]);
        status.setAttribute("role", "img");
        status.setAttribute("aria-label", label);
        status.title = label;
        row.append(status, BinaryDiagram.node("small", "binary-function-label", label.toLowerCase()));
        if (fn.source) {
          const link = BinaryDiagram.node("a", "binary-source-link", "🔗");
          link.href = fn.source;
          link.target = "_blank";
          link.rel = "noopener noreferrer";
          link.setAttribute("aria-label", "Open function source on main");
          link.title = "Open function source on main";
          row.append(link);
        }
        tooltip.append(row);
      }
    } else {
      tooltip.append(BinaryDiagram.node("div", "binary-detail", span.detail));
    }
    if (span.preview !== undefined) {
      const preview = BinaryDiagram.node("div", "binary-preview", span.preview);
      tooltip.insertBefore(preview, tooltip.lastChild);
    }
    tooltip.hidden = false;
    this.canvas.setAttribute("aria-describedby", tooltip.id);
    this.dataset.selectedStart = span.start;
    this.dataset.selectedEnd = span.end;
    this.dataset.selectedCategory = span.category;
    const rect = this.canvas.getBoundingClientRect();
    const left = event ? event.clientX - rect.left + 12 : 8;
    tooltip.style.left = `${left}px`;
    const y = event ? (event.clientY - rect.top) / rect.height
      : (Math.floor(span.start / this.binary.size * BinaryDiagram.rows) + .5) / BinaryDiagram.rows;
    this.tooltipBand = Math.max(0, Math.min(2, Math.floor(y * 3)));
    this.updateNavigation();
    this.draw();
  }

  clear() {
    this.selected = -1;
    this.pinned = false;
    if (BinaryDiagram.active === this) {
      BinaryDiagram.active = null;
      BinaryDiagram.tooltip.hidden = true;
      document.body.append(BinaryDiagram.tooltip);
    }
    this.canvas.removeAttribute("aria-describedby");
    delete this.dataset.selectedStart;
    delete this.dataset.selectedEnd;
    delete this.dataset.selectedCategory;
    this.updateNavigation();
    this.draw();
  }

  navigate(step) {
    const next = this.findVisible(this.selected, step);
    if (next < 0) return;
    this.pinned = true;
    this.select(next);
  }

  key(event) {
    if (event.key === "Escape") {
      event.preventDefault();
      if (BinaryDiagram.tooltip.contains(event.target)) this.canvas.focus();
      this.clear();
      return;
    }
    let next;
    if (event.key === "Home") next = this.findVisible(-1, 1);
    else if (event.key === "End") next = this.findVisible(this.binary.spans.length, -1);
    else if (["ArrowRight", "ArrowDown"].includes(event.key)) next = this.findVisible(this.selected, 1);
    else if (["ArrowLeft", "ArrowUp"].includes(event.key)) next = this.findVisible(this.selected, -1);
    else return;
    event.preventDefault();
    if (BinaryDiagram.tooltip.contains(event.target)) this.canvas.focus();
    if (next < 0) return;
    this.pinned = true;
    this.select(next);
  }

  rectangles(span, width, height, paint) {
    const first = span.start / this.binary.size * BinaryDiagram.rows;
    const end = span.end / this.binary.size * BinaryDiagram.rows;
    for (let row = Math.floor(first); row < Math.ceil(end); row++) {
      const x = Math.max(0, first - row) * width;
      const right = Math.min(1, end - row) * width;
      paint(x, row * height / BinaryDiagram.rows, right - x, height / BinaryDiagram.rows);
    }
  }

  draw() {
    if (BinaryDiagram.helpOwner === this) this.positionHelp();
    if (BinaryDiagram.filterOwner === this) this.positionFilter();
    if (!this.visible) return;
    const { width, height } = this.canvas.getBoundingClientRect();
    if (!width || !height) return;
    const ratio = window.devicePixelRatio || 1;
    this.canvas.width = Math.round(width * ratio);
    this.canvas.height = Math.round(height * ratio);
    const context = this.canvas.getContext("2d");
    context.scale(ratio, ratio);
    for (const span of this.binary.spans) {
      if (this.hiddenCategories.has(span.category)) continue;
      let color = this.categories.find((row) => row.id === span.category).color;
      if (span === this.binary.spans[this.selected]) {
        color = color.replace(/[0-9a-f]{2}/gi, (channel) =>
          Math.round(parseInt(channel, 16) * .65 + 255 * .35).toString(16).padStart(2, "0"));
      }
      context.fillStyle = color;
      this.rectangles(span, width, height, (x, y, w, h) => context.fillRect(x, y, w, h));
    }
    if (BinaryDiagram.active === this) {
      const tooltip = BinaryDiagram.tooltip;
      const gutter = 6;
      tooltip.style.left = `${Math.max(gutter, Math.min(parseFloat(tooltip.style.left) || gutter, width - tooltip.offsetWidth - gutter))}px`;
      tooltip.style.top = `${8 + this.tooltipBand / 2 * Math.max(0, height - tooltip.offsetHeight - 16)}px`;
    }
  }
}

customElements.define("binary-diagram", BinaryDiagram);
