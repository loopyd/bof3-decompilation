"use strict";

class BinaryDiagram extends HTMLElement {
  static rows = 32;
  static active = null;
  static tooltip = null;
  static helpOwner = null;
  static helpTooltip = null;

  static node(tag, className, text) {
    const result = document.createElement(tag);
    result.className = className;
    if (text !== undefined) result.textContent = text;
    return result;
  }

  static isHelpTarget(target) {
    return target?.closest?.(".binary-help-button") || this.helpTooltip?.contains(target);
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
    tooltip.addEventListener("pointerdown", (event) => {
      if (event.target.closest(".binary-source-link")) event.preventDefault();
    });
    tooltip.addEventListener("pointerleave", (event) => {
      const active = this.active;
      if (active && !active.pinned && event.relatedTarget !== active.canvas) active.clear();
    });
    document.body.append(tooltip);
    this.tooltip = tooltip;
    const dismissOutside = (event) => {
      const active = this.active;
      if (!active || event.target === active.canvas || active.navigation.contains(event.target) || tooltip.contains(event.target) || this.isHelpTarget(event.target)) return;
      active.clear();
      active.navigation.hidden = true;
      if (active.contains(document.activeElement)) document.activeElement.blur();
    };
    document.addEventListener("pointerdown", dismissOutside, true);
    document.addEventListener("click", dismissOutside, true);
    document.addEventListener("focusin", (event) => {
      const active = this.active;
      if (!active || active.contains(event.target) || this.isHelpTarget(event.target)) return;
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
    const node = BinaryDiagram.node;
    this.surface = node("div", "binary-surface");
    this.canvas = node("canvas", "binary-canvas");
    this.canvas.tabIndex = 0;
    this.canvas.setAttribute("role", "img");
    this.canvas.setAttribute("aria-label", `${this.target}: original binary byte map. Arrow keys select spans; Home and End select bounds; Escape clears selection.`);
    BinaryDiagram.sharedTooltip();
    BinaryDiagram.sharedHelp();
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
    heading.append(node("p", "metric-label", "Binary layout"), this.helpButton);
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
    this.replaceChildren(heading, this.surface, bounds);
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
      if (this.surface.contains(event.target)) this.key(event);
    }, { signal: this.events.signal });
    this.addEventListener("focusin", (event) => {
      if (!this.surface.contains(event.target) || this.surface.contains(event.relatedTarget)) return;
      this.navigation.hidden = false;
      this.pinned = true;
      if (BinaryDiagram.tooltip.contains(event.target)) this.draw();
      else this.select(Math.max(0, this.selected));
    }, { signal: this.events.signal });
    this.addEventListener("focusout", (event) => {
      if (this.contains(event.relatedTarget)) return;
      this.navigation.hidden = true;
      if (BinaryDiagram.isHelpTarget(event.relatedTarget)) this.draw();
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
    this.closeHelp();
    if (this.canvas) this.clear();
    this.resize?.disconnect();
    this.visibility?.disconnect();
    this.events?.abort();
  }

  showHelp() {
    BinaryDiagram.helpOwner?.closeHelp();
    const node = BinaryDiagram.node;
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
    const tooltip = BinaryDiagram.helpTooltip;
    tooltip.replaceChildren(node("p", "metric-label", "Binary legend"), legend);
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
    const tooltip = BinaryDiagram.helpTooltip;
    const button = this.helpButton.getBoundingClientRect();
    const canvas = this.canvas.getBoundingClientRect();
    tooltip.style.width = `${Math.min(280, canvas.width, window.innerWidth - 16)}px`;
    const height = tooltip.offsetHeight;
    const top = button.top >= height + 8 ? button.top - height - 8 : canvas.bottom + 8;
    tooltip.style.top = `${Math.max(8, Math.min(top, window.innerHeight - height - 8))}px`;
    tooltip.style.left = `${Math.max(8, Math.min(canvas.right - tooltip.offsetWidth, window.innerWidth - tooltip.offsetWidth - 8))}px`;
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
    const bounds = `${this.hex(span.start)}–${this.hex(span.end - 1)} · ${(span.end - span.start).toLocaleString("en-US")} bytes`;
    const tooltip = BinaryDiagram.tooltip;
    tooltip.replaceChildren();
    if (span.functions) {
      const summary = BinaryDiagram.node("div", "binary-function-summary");
      summary.append(
        BinaryDiagram.node("span", "", category.label),
        BinaryDiagram.node("small", "binary-function-range", bounds),
      );
      tooltip.append(summary);
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
      tooltip.append(document.createTextNode(`${category.label} · ${bounds}`), document.createElement("br"), document.createTextNode(span.detail));
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
    this.previous.setAttribute("aria-disabled", index === 0);
    this.next.setAttribute("aria-disabled", index === this.binary.spans.length - 1);
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
    this.draw();
  }

  navigate(step) {
    this.pinned = true;
    this.select(Math.max(0, Math.min(this.binary.spans.length - 1, this.selected + step)));
  }

  key(event) {
    if (event.key === "Escape") {
      event.preventDefault();
      if (BinaryDiagram.tooltip.contains(event.target)) this.canvas.focus();
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
    if (BinaryDiagram.tooltip.contains(event.target)) this.canvas.focus();
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
      tooltip.style.top = `${8 + this.tooltipBand / 2 * Math.max(0, height - tooltip.offsetHeight - 16)}px`;
    }
  }
}

customElements.define("binary-diagram", BinaryDiagram);
