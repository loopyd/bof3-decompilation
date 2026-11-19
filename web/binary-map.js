"use strict";

class BinaryDiagram extends HTMLElement {
  static rows = 32;
  static sequence = 0;

  configure(binary, categories, target) {
    this.binary = binary;
    this.categories = categories;
    this.target = target;
  }

  connectedCallback() {
    if (!this.binary) return;
    this.selected = -1;
    this.pinned = false;
    const node = (tag, className, text) => {
      const result = document.createElement(tag);
      result.className = className;
      if (text !== undefined) result.textContent = text;
      return result;
    };
    const surface = node("div", "binary-surface");
    this.canvas = node("canvas", "binary-canvas");
    this.canvas.tabIndex = 0;
    this.canvas.setAttribute("role", "img");
    this.canvas.setAttribute("aria-label", `${this.target}: original binary byte map. Arrow keys select spans; Home and End select bounds; Escape clears selection.`);
    this.tooltip = node("div", "binary-tooltip");
    this.tooltip.id = `binary-tooltip-${++BinaryDiagram.sequence}`;
    this.tooltip.setAttribute("role", "tooltip");
    this.tooltip.setAttribute("aria-live", "polite");
    this.tooltip.hidden = true;
    surface.append(this.canvas, this.tooltip);
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
    this.replaceChildren(node("p", "metric-label", "Binary layout"), surface, bounds, legend);
    this.canvas.addEventListener("pointermove", (event) => {
      if (!this.pinned) this.select(this.hit(event), event);
    });
    this.canvas.addEventListener("pointerleave", () => {
      if (!this.pinned) this.clear();
    });
    this.canvas.addEventListener("click", (event) => {
      this.pinned = true;
      this.select(this.hit(event), event);
    });
    this.canvas.addEventListener("keydown", (event) => this.key(event));
    this.resize = new ResizeObserver(() => this.draw());
    this.resize.observe(this.canvas);
    this.visibility = new IntersectionObserver(([entry]) => {
      this.visible = entry.isIntersecting;
      if (this.visible) this.draw();
    });
    this.visibility.observe(this);
    this.events = new AbortController();
    this.watchDensity();
  }

  disconnectedCallback() {
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
    this.selected = index;
    const span = this.binary.spans[index];
    const category = this.categories.find((row) => row.id === span.category);
    const text = `${category.label} · ${this.hex(span.start)}–${this.hex(span.end - 1)} · ${(span.end - span.start).toLocaleString("en-US")} bytes`;
    this.tooltip.replaceChildren(document.createTextNode(text), document.createElement("br"), document.createTextNode(span.detail));
    this.tooltip.hidden = false;
    this.canvas.setAttribute("aria-describedby", this.tooltip.id);
    this.dataset.selectedStart = span.start;
    this.dataset.selectedEnd = span.end;
    this.dataset.selectedCategory = span.category;
    const rect = this.canvas.getBoundingClientRect();
    const left = event ? event.clientX - rect.left + 12 : 8;
    this.tooltip.style.left = `${Math.max(6, Math.min(left, rect.width - this.tooltip.offsetWidth - 6))}px`;
    this.draw();
  }

  clear() {
    this.selected = -1;
    this.pinned = false;
    this.tooltip.hidden = true;
    this.canvas.removeAttribute("aria-describedby");
    delete this.dataset.selectedStart;
    delete this.dataset.selectedEnd;
    delete this.dataset.selectedCategory;
    this.draw();
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
      context.fillStyle = this.categories.find((row) => row.id === span.category).color;
      this.rectangles(span, width, height, (x, y, w, h) => context.fillRect(x, y, w, h));
    }
    if (this.selected >= 0) {
      context.strokeStyle = "#fff";
      context.lineWidth = 1.5;
      this.rectangles(this.binary.spans[this.selected], width, height, (x, y, w, h) => context.strokeRect(x + 0.75, y + 0.75, Math.max(1, w - 1.5), h - 1.5));
    }
    const tooltipWidth = this.tooltip.offsetWidth;
    if (tooltipWidth) this.tooltip.style.left = `${Math.max(6, Math.min(parseFloat(this.tooltip.style.left) || 6, width - tooltipWidth - 6))}px`;
  }
}

customElements.define("binary-diagram", BinaryDiagram);
