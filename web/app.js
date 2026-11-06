"use strict";

const states = ["completed", "partial", "unlifted"];
const symbolStates = ["named", "raw"];
const format = new Intl.NumberFormat("en-US");
const byId = (id) => document.getElementById(id);
const percent = (count, total) => total ? `${(100 * count / total).toFixed(1)}%` : "—";
let targets = [];
let selectedKind = "exe";
const collapsedFamilies = new Set();

function element(tag, className, text) {
  const node = document.createElement(tag);
  if (className) node.className = className;
  if (text !== undefined) node.textContent = text;
  return node;
}

function progressBar(target, categories = states, noun = "functions") {
  const bar = element("div", "bar");
  bar.setAttribute("role", "img");
  bar.setAttribute("aria-label", target.total
    ? `Indexed ${noun}: ` + categories.map((state) => `${format.format(target[state])} ${state}, ${percent(target[state], target.total)}`).join("; ")
    : `No indexed ${noun}`);
  for (const state of categories) {
    const segment = element("span", state);
    segment.style.width = `${target.total ? 100 * target[state] / target.total : 0}%`;
    bar.append(segment);
  }
  return bar;
}

function counts(target, categories = states) {
  const row = element("div", "target-counts");
  for (const state of categories) {
    const label = element("span");
    label.append(element("i", state), element("strong", "", format.format(target[state])), ` ${state}`);
    row.append(label);
  }
  return row;
}

function symbolProgress(symbols) {
  const section = element("div", "symbol-progress");
  const label = element("div", "metric-label");
  label.append(element("span", "", "Symbols"), element("span", "", symbols.total
    ? `${percent(symbols.named, symbols.total)} named`
    : "No indexed symbols"));
  section.append(label, progressBar(symbols, symbolStates, "symbols"), counts(symbols, symbolStates));
  return section;
}

function executable(target) {
  const article = element("article", "executable");
  const heading = element("div", "target-heading");
  const identity = element("div", "target-identity");
  const name = element("div");
  const label = target.archive.replace(/_(\d{3})_(\d{2})$/, "_$1.$2").toUpperCase();
  name.append(element("h2", "target-title", label), element("p", "target-path", target.id));
  identity.append(element("span", "target-icon", "EXE"), name);
  const completion = element("div", "target-percent", percent(target.completed, target.total));
  completion.append(element("small", "", target.total ? "completed" : "no indexed functions"));
  heading.append(identity, completion);
  article.append(heading, element("div", "metric-label", "Decompilation"), progressBar(target), counts(target), symbolProgress(target.symbols));
  return article;
}

function emiCard(target) {
  const article = element("article", "target-card");
  const heading = element("div", "card-top");
  heading.append(element("h3", "target-title", `${target.archive.toUpperCase()}.EMI`), element("span", "entry", `#${target.entry}`));
  const progress = element("div", "card-progress");
  const value = element("span", "percent", percent(target.completed, target.total));
  value.title = target.total ? "Completed indexed functions" : "No indexed functions";
  progress.append(progressBar(target), value);
  article.append(heading, element("p", "target-path", target.id), element("div", "metric-label decomp-label", "Decompilation"), progress, counts(target));
  if (!target.total) article.append(element("p", "target-path", "No indexed functions"));
  article.append(symbolProgress(target.symbols));
  return article;
}

function emiFamily(name, members) {
  const section = element("section", "family-group");
  const heading = element("h2", "family-heading");
  const button = element("button");
  button.type = "button";
  const grid = element("div", "card-grid");
  grid.id = `emi-family-${encodeURIComponent(name)}`;
  grid.hidden = collapsedFamilies.has(name);
  button.setAttribute("aria-controls", grid.id);
  button.setAttribute("aria-expanded", String(!grid.hidden));
  button.append(element("span", "family-name", name.toUpperCase()), element("span", "family-count", `${members.length} targets`));
  button.addEventListener("click", () => {
    grid.hidden = !grid.hidden;
    button.setAttribute("aria-expanded", String(!grid.hidden));
    if (grid.hidden) collapsedFamilies.add(name);
    else collapsedFamilies.delete(name);
  });
  heading.append(button);
  grid.append(...members.map(emiCard));
  section.append(heading, grid);
  return section;
}

function renderTargets() {
  const query = byId("search").value.trim().toLowerCase();
  const family = byId("family").value;
  const selected = targets.filter((target) => target.kind === selectedKind
    && target.id.toLowerCase().includes(query)
    && (selectedKind === "exe" || !family || target.family === family));
  const panel = byId(`panel-${selectedKind}`);
  panel.replaceChildren();
  if (selectedKind === "exe") panel.append(...selected.map(executable));
  if (selectedKind === "emi") {
    for (const name of [...new Set(selected.map((target) => target.family))]) {
      const members = selected.filter((target) => target.family === name);
      panel.append(emiFamily(name, members));
    }
  }
  byId("empty").hidden = selected.length > 0;
  byId("results-count").textContent = `${format.format(selected.length)} ${selectedKind === "exe" ? "executable" : "EMI"} targets · ${format.format(selected.reduce((total, target) => total + target.total, 0))} indexed functions`;
}

function selectTab(kind, focus = false) {
  selectedKind = kind;
  for (const name of ["exe", "emi"]) {
    const selected = kind === name;
    byId(`tab-${name}`).setAttribute("aria-selected", String(selected));
    byId(`tab-${name}`).tabIndex = selected ? 0 : -1;
    byId(`panel-${name}`).hidden = !selected;
  }
  byId("family-label").hidden = kind !== "emi";
  renderTargets();
  if (focus) byId(`tab-${kind}`).focus();
}

function validateSnapshot(snapshot) {
  if (snapshot.schema !== "bof3.decomp-status/v2" || !/^[0-9a-f]{40,64}$/.test(snapshot.revision)
    || !Array.isArray(snapshot.targets) || !snapshot.targets.length || !Number.isFinite(Date.parse(snapshot.published_at))) {
    throw new Error("The progress snapshot has an unsupported format.");
  }
  const seen = new Set();
  const countKeys = [...states, "total", "invalid"];
  for (const row of [...snapshot.targets, snapshot.counts]) {
    if (!row || countKeys.some((key) => !Number.isSafeInteger(row[key]) || row[key] < 0)
      || states.reduce((total, key) => total + row[key], 0) !== row.total || row.invalid > row.unlifted) {
      throw new Error("The progress snapshot contains inconsistent counts.");
    }
    const symbols = row.symbols;
    if (!symbols || [...symbolStates, "total"].some((key) => !Number.isSafeInteger(symbols[key]) || symbols[key] < 0)
      || symbols.named + symbols.raw !== symbols.total) {
      throw new Error("The progress snapshot contains inconsistent symbol counts.");
    }
  }
  for (const target of snapshot.targets) {
    if (!["exe", "emi"].includes(target.kind) || typeof target.id !== "string"
      || !target.id.startsWith(`${target.kind}/`) || seen.has(target.id)
      || typeof target.family !== "string" || typeof target.archive !== "string") {
      throw new Error("The progress snapshot contains an invalid target.");
    }
    seen.add(target.id);
  }
  if (countKeys.some((key) => snapshot.targets.reduce((total, row) => total + row[key], 0) !== snapshot.counts[key])) {
    throw new Error("The progress totals do not match the targets.");
  }
  if ([...symbolStates, "total"].some((key) => snapshot.targets.reduce((total, row) => total + row.symbols[key], 0) !== snapshot.counts.symbols[key])) {
    throw new Error("The symbol totals do not match the targets.");
  }
}

function renderOverview(snapshot) {
  byId("overall-percent").textContent = percent(snapshot.counts.completed, snapshot.counts.total);
  byId("overall-fraction").textContent = `${format.format(snapshot.counts.completed)} of ${format.format(snapshot.counts.total)} functions`;
  byId("overall-bar").replaceChildren(progressBar(snapshot.counts));
  for (const state of states) {
    const stat = element("div", "stat");
    const label = element("span", "stat-label");
    label.append(element("i", state), state[0].toUpperCase() + state.slice(1));
    stat.append(label, element("strong", "", format.format(snapshot.counts[state])), element("small", "", percent(snapshot.counts[state], snapshot.counts.total)));
    byId("overview-stats").append(stat);
  }
  const link = element("a", "", snapshot.revision.slice(0, 7));
  link.href = `https://github.com/loopyd/bof3-decompilation/commit/${snapshot.revision}`;
  const date = new Date(snapshot.published_at).toLocaleDateString("en-US", { year: "numeric", month: "short", day: "numeric", timeZone: "UTC" });
  byId("snapshot-label").replaceChildren(`Index snapshot · ${date} UTC · `, link);
  byId("invalid-note").textContent = `${format.format(snapshot.counts.invalid)} functions have invalid lift metadata and are included in unlifted. Targets without indexed functions remain visible with no completion percentage.`;
}

async function initialize() {
  try {
    const response = await fetch("status.json", { cache: "no-cache" });
    if (!response.ok) throw new Error(`Snapshot request failed (${response.status}).`);
    const snapshot = await response.json();
    validateSnapshot(snapshot);
    targets = snapshot.targets;
    renderOverview(snapshot);
    for (const kind of ["exe", "emi"]) {
      byId(`${kind}-count`).textContent = targets.filter((target) => target.kind === kind).length;
      byId(`tab-${kind}`).addEventListener("click", () => selectTab(kind));
      byId(`tab-${kind}`).addEventListener("keydown", (event) => {
        if (!["ArrowLeft", "ArrowRight", "Home", "End"].includes(event.key)) return;
        event.preventDefault();
        selectTab(event.key === "Home" ? "exe" : event.key === "End" ? "emi" : selectedKind === "exe" ? "emi" : "exe", true);
      });
    }
    for (const family of [...new Set(targets.filter((target) => target.kind === "emi").map((target) => target.family))]) {
      const option = element("option", "", family.toUpperCase());
      option.value = family;
      byId("family").append(option);
    }
    byId("search").addEventListener("input", renderTargets);
    byId("family").addEventListener("change", renderTargets);
    byId("dashboard").hidden = false;
    selectTab("exe");
  } catch (error) {
    byId("dashboard").hidden = true;
    byId("error").hidden = false;
    byId("error").textContent = `Progress is unavailable. ${error.message} Try reloading the page.`;
    byId("snapshot-label").textContent = "Reverse-index snapshot unavailable";
  }
}

initialize();
