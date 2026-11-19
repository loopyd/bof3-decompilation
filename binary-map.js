"use strict";

class BinaryDiagram extends HTMLElement {
  static rows = 32;
  static active = null;
  static tooltip = null;

  static sharedTooltip() {
    if (this.tooltip) return this.tooltip;
    const tooltip = document.createElement("div");
    tooltip.className = "binary-tooltip";
    tooltip.id = "binary-span-tooltip";
    tooltip.setAttribute("role", "tooltip");
    tooltip.setAttribute("aria-live", "polite");
    tooltip.hidden = true;
    document.body.append(tooltip);
    this.tooltip = tooltip;
    const dismissOutside = (event) => {
      const active = this.active;
      if (!active || event.target === active.canvas || active.navigation.contains(event.target)) return;
      active.clear();
      active.navigation.hidden = true;
      if (active.contains(document.activeElement)) document.activeElement.blur();
    };
    document.addEventListener("pointerdown", dismissOutside, true);
    document.addEventListener("click", dismissOutside, true);
    return tooltip;
  }

  configure(binary, categories, target) {
    this.binary = binary;
    this.categories = categories;
    this.target = target;
  }

  connectedCallback() {
    if (!this.binary) return;
    this.selected = -1;
    this.pinned = false;
    this.events = new AbortController();
    const node = (tag, className, text) => {
      const result = document.createElement(tag);
      result.className = className;
      if (text !== undefined) result.textContent = text;
      return result;
    };
    this.surface = node("div", "binary-surface");
    this.canvas = node("canvas", "binary-canvas");
    this.canvas.tabIndex = 0;
    this.canvas.setAttribute("role", "img");
    this.canvas.setAttribute("aria-label", `${this.target}: original binary byte map. Arrow keys select spans; Home and End select bounds; Escape clears selection.`);
    BinaryDiagram.sharedTooltip();
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
    this.surface.append(this.canvas, this.navigation);
    const bounds = node("p", "binary-bounds");
    bounds.append(node("span", "", "0x000000"), node("span", "", `${this.hex(this.binary.size - 1)} · ${this.binary.size.toLocaleString("en-US")} bytes`));
    const legend = node("ul", "binary-legend");
    legend.setAttribute("aria-label", "Binary classification legend");
    for (const category of this.categories) {
      const item = node("li", "");
      const chip = node("i", "binary-chip");
      chip.style.backgroundColor = category.color;
      chip.setAttribute("aria-hidden", "true");
      item.append(chip, category.label);
      legend.append(item);
    }
    this.replaceChildren(node("p", "metric-label", "Binary layout"), this.surface, bounds, legend);
    this.canvas.addEventListener("pointermove", (event) => {
      if (!this.pinned && !BinaryDiagram.active?.pinned) this.select(this.hit(event), event);
    });
    this.canvas.addEventListener("pointerleave", () => {
      if (!this.pinned) this.clear();
    });
    this.canvas.addEventListener("click", (event) => {
      this.pinned = true;
      this.select(this.hit(event), event);
    });
    this.addEventListener("keydown", (event) => this.key(event), { signal: this.events.signal });
    this.addEventListener("focusin", (event) => {
      if (this.contains(event.relatedTarget)) return;
      this.navigation.hidden = false;
      this.pinned = true;
      this.select(Math.max(0, this.selected));
    }, { signal: this.events.signal });
    this.addEventListener("focusout", (event) => {
      if (this.contains(event.relatedTarget)) return;
      this.navigation.hidden = true;
      this.clear();
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
    if (this.canvas) this.clear();
    this.resize?.disconnect();
    this.visibility?.disconnect();
    this.events?.abort();
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
    const row = Math.min(BinaryDiagram.rows - 1, Math.max(0, Math.floor((event.clientY - rect.top) / rect.height * BinaryDiagram.rows)));
    const x = Math.max(0, Math.min(1, (event.clientX - rect.left) / rect.width));
    const offset = Math.min(this.binary.size - 1, Math.floor((row + x) / BinaryDiagram.rows * this.binary.size));
    let low = 0;
    let high = this.binary.spans.length - 1;
    while (low < high) {
      const middle = Math.floor((low + high) / 2);
      if (this.binary.spans[middle].end <= offset) low = middle + 1;
      else high = middle;
    }
    return low;
  }

  select(index, event) {
    if (BinaryDiagram.active !== this) {
      BinaryDiagram.active?.clear();
      BinaryDiagram.active = this;
      this.surface.append(BinaryDiagram.tooltip);
    }
    this.selected = index;
    const span = this.binary.spans[index];
    const category = this.categories.find((row) => row.id === span.category);
    const text = `${category.label} · ${this.hex(span.start)}–${this.hex(span.end - 1)} · ${(span.end - span.start).toLocaleString("en-US")} bytes`;
    const tooltip = BinaryDiagram.tooltip;
    tooltip.replaceChildren(document.createTextNode(text), document.createElement("br"), document.createTextNode(span.detail));
    tooltip.hidden = false;
    this.canvas.setAttribute("aria-describedby", tooltip.id);
    this.dataset.selectedStart = span.start;
    this.dataset.selectedEnd = span.end;
    this.dataset.selectedCategory = span.category;
    const rect = this.canvas.getBoundingClientRect();
    const left = event ? event.clientX - rect.left + 12 : 8;
    tooltip.style.left = `${left}px`;
    this.previous.setAttribute("aria-disabled", index === 0);
    this.next.setAttribute("aria-disabled", index === this.binary.spans.length - 1);
    this.draw();
  }

  clear() {
    this.selected = -1;
    this.pinned = false;
    if (BinaryDiagram.active === this) {
      BinaryDiagram.tooltip.hidden = true;
      document.body.append(BinaryDiagram.tooltip);
      BinaryDiagram.active = null;
    }
    this.canvas.removeAttribute("aria-describedby");
    delete this.dataset.selectedStart;
    delete this.dataset.selectedEnd;
    delete this.dataset.selectedCategory;
    this.draw();
  }

  navigate(step) {
    this.pinned = true;
    this.select(Math.max(0, Math.min(this.binary.spans.length - 1, this.selected + step)));
  }

  key(event) {
    if (event.key === "Escape") {
      event.preventDefault();
      this.clear();
      return;
    }
    const last = this.binary.spans.length - 1;
    let next;
    if (event.key === "Home") next = 0;
    else if (event.key === "End") next = last;
    else if (["ArrowRight", "ArrowDown"].includes(event.key)) next = Math.min(last, this.selected + 1);
    else if (["ArrowLeft", "ArrowUp"].includes(event.key)) next = Math.max(0, this.selected - 1);
    else return;
    event.preventDefault();
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
    if (!this.visible) return;
    const { width, height } = this.canvas.getBoundingClientRect();
    if (!width || !height) return;
    const ratio = window.devicePixelRatio || 1;
    this.canvas.width = Math.round(width * ratio);
    this.canvas.height = Math.round(height * ratio);
    const context = this.canvas.getContext("2d");
    context.scale(ratio, ratio);
    for (const span of this.binary.spans) {
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
      const gutter = this.navigation.hidden ? 6 : 50;
      tooltip.style.left = `${Math.max(gutter, Math.min(parseFloat(tooltip.style.left) || gutter, width - tooltip.offsetWidth - gutter))}px`;
    }
  }
}

customElements.define("binary-diagram", BinaryDiagram);
