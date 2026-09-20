-- ============================================================
-- Anaconda XB 360 - Store Client
-- Runs inside Aurora on a modded Xbox 360.
-- Reads repo.ini, shows categories, downloads and installs
-- homebrew apps, games, emulators and themes.
-- ============================================================

Anaconda = {}

Anaconda.Name        = "Anaconda XB 360"
Anaconda.Version     = "1.0.0"
Anaconda.Author      = "H3X Anaconda Team"
Anaconda.DefaultRepo = "https://raw.githubusercontent.com/H3X-Anaconda-Team/anaconda-xb-360/main/repo/repo.ini"

Anaconda.BaseDir     = "Hdd:\\Anaconda\\"
Anaconda.CacheDir    = Anaconda.BaseDir .. "cache\\"
Anaconda.ConfigFile  = Anaconda.BaseDir .. "anaconda.cfg"
Anaconda.TempArchive = Anaconda.CacheDir .. "package.7z"
Anaconda.LogFile     = Anaconda.BaseDir .. "anaconda.log"

Anaconda.RepoUrl     = Anaconda.DefaultRepo
Anaconda.Categories  = {}
Anaconda.Packages    = {}

-- ============================================================
-- SECTION 0 : AURORA API LAYER
-- Every call that touches Aurora internals lives here.
-- If your Aurora build uses different function names, only
-- edit this block. The rest of the script stays the same.
-- ============================================================

local function Log(msg)
  local line = os.date("[%Y-%m-%d %H:%M:%S] ") .. tostring(msg) .. "\n"
  local f = io.open(Anaconda.LogFile, "a")
  if f then f:write(line) f:close() end
end

local function EnsureDir(path)
  if not FileSystem.DirectoryExists(path) then
    FileSystem.CreateDirectory(path)
  end
end

local function FileExists(path)
  return FileSystem.FileExists(path)
end

local function ReadText(path)
  if not FileExists(path) then return nil end
  return FileSystem.ReadFile(path)
end

local function WriteText(path, data)
  FileSystem.WriteFile(path, data)
end

local function HttpGet(url)
  local status, body = Http.Get(url)
  if status ~= 200 and status ~= 0 then
    return nil, "HTTP " .. tostring(status)
  end
  return body
end

local function HttpDownload(url, dest)
  local body, err = HttpGet(url)
  if not body then return false, err end
  WriteText(dest, body)
  return true
end

local function Extract7z(archive, destPath)
  return FileSystem.Extract(archive, destPath)
end

local function ShowMessage(title, text)
  Aurora.ShowMessageBox(title, text, "OK")
end

local function ShowKeyboard(prompt, default)
  return Aurora.ShowKeyboard(prompt, default or "")
end

local function NewMenu()
  return Script.NewMenu()
end

local function AddItem(menu, id, text)
  Script.AddMenuItem(menu, id, text)
end

local function ShowMenu(menu)
  return Script.ShowMenu(menu)
end

local function ReloadAurora()
  Aurora.Reload()
end

-- ============================================================
-- SECTION 1 : CONFIG
-- ============================================================

function Anaconda.LoadConfig()
  EnsureDir(Anaconda.BaseDir)
  EnsureDir(Anaconda.CacheDir)

  local data = ReadText(Anaconda.ConfigFile)
  if data then
    local url = data:match("repoUrl%s*=%s*(.+)")
    if url then
      Anaconda.RepoUrl = url:gsub("%s+$", "")
    end
  else
    Anaconda.SaveConfig()
  end
end

function Anaconda.SaveConfig()
  WriteText(Anaconda.ConfigFile,
    "repoUrl=" .. Anaconda.RepoUrl .. "\n")
end

-- ============================================================
-- SECTION 2 : INI PARSER
-- Parses .ini files into:
-- { Sections = { [name] = { key = value } }, Order = { name, ... } }
-- ============================================================

function Anaconda.ParseIni(text)
  local result = { Sections = {}, Order = {} }
  local current = nil

  for line in text:gmatch("[^\r\n]+") do
    line = line:match("^%s*(.-)%s*$")

    if line ~= "" and not line:match("^[;%#]") then
      local section = line:match("^%[(.+)%]$")
      if section then
        current = section
        if not result.Sections[current] then
          result.Sections[current] = {}
          table.insert(result.Order, current)
        end
      elseif current then
        local key, value = line:match("^([^=]+)=(.*)$")
        if key then
          result.Sections[current][key:match("^%s*(.-)%s*$")] = value
        end
      end
    end
  end

  return result
end

-- ============================================================
-- SECTION 3 : REPO FETCHING
-- ============================================================

function Anaconda.FetchIni(url)
  local body, err = HttpGet(url)
  if not body then
    Log("Fetch failed: " .. url .. " (" .. tostring(err) .. ")")
    return nil
  end
  return Anaconda.ParseIni(body)
end

function Anaconda.LoadCategories()
  local ini = Anaconda.FetchIni(Anaconda.RepoUrl)
  if not ini then return nil end

  local localCats = {}
  local externalCats = {}

  for _, name in ipairs(ini.Order) do
    local s = ini.Sections[name]
    if s.iniurl then
      if name:find("Free60") or name:find("X-Store") or name:find("External") then
        table.insert(externalCats, { name = name, url = s.iniurl })
      else
        table.insert(localCats, { name = name, url = s.iniurl })
      end
    end
  end

  for _, cat in ipairs(externalCats) do
    table.insert(localCats, cat)
  end

  Anaconda.Categories = localCats
  return localCats
end

function Anaconda.LoadPackages(url)
  local ini = Anaconda.FetchIni(url)
  if not ini then return nil end

  local pkgs = {}
  for _, name in ipairs(ini.Order) do
    local s = ini.Sections[name]
    if s.dataurl then
      table.insert(pkgs, {
        id          = name,
        title       = s.itemTitle or name,
        version     = s.itemVersion or "?",
        author      = s.itemAuthor or "Unknown",
        description = s.itemDescription or "",
        dataurl     = s.dataurl,
        path        = s.path or ("/Apps/" .. name .. "/"),
        reload      = (s.reload == "True")
      })
    end
  end

  Anaconda.Packages = pkgs
  return pkgs
end

-- ============================================================
-- SECTION 4 : INSTALLER
-- ============================================================

function Anaconda.Install(pkg)
  ShowMessage(Anaconda.Name,
    "Downloading " .. pkg.title .. "...")

  EnsureDir(Anaconda.CacheDir)

  local ok, err = HttpDownload(pkg.dataurl, Anaconda.TempArchive)
  if not ok then
    ShowMessage("Download failed", tostring(err))
    Log("Download failed: " .. pkg.title .. " - " .. tostring(err))
    return false
  end

  local dest = "Hdd:" .. pkg.path:gsub("/", "\\")
  if dest:sub(-1) ~= "\\" then dest = dest .. "\\" end
  EnsureDir(dest)

  local extracted = Extract7z(Anaconda.TempArchive, dest)
  if not extracted then
    ShowMessage("Extract failed", "Could not unpack " .. pkg.title)
    Log("Extract failed: " .. pkg.title)
    return false
  end

  FileSystem.DeleteFile(Anaconda.TempArchive)

  if pkg.reload then
    ReloadAurora()
  end

  ShowMessage("Installed", pkg.title .. " installed to " .. dest)
  Log("Installed: " .. pkg.title .. " -> " .. dest)
  return true
end

-- ============================================================
-- SECTION 5 : UI
-- ============================================================

function Anaconda.CategoryMenu(cats)
  local menu = NewMenu()

  AddItem(menu, -1, "=== " .. Anaconda.Name .. " v" .. Anaconda.Version .. " ===")
  AddItem(menu, 0, "Settings")
  AddItem(menu, -2, "-----------------------------")

  for i, c in ipairs(cats) do
    local label = c.name
    if c.name:find("Free60") then
      label = "[Free60] " .. c.name
    end
    AddItem(menu, i, label)
  end

  return ShowMenu(menu)
end

function Anaconda.PackageMenu(pkgs, title)
  local menu = NewMenu()

  AddItem(menu, -1, "=== " .. title .. " (" .. #pkgs .. " items) ===")

  for i, p in ipairs(pkgs) do
    AddItem(menu, i, p.title .. "  v" .. p.version .. "  by " .. p.author)
  end

  return ShowMenu(menu)
end

function Anaconda.Settings()
  local url = ShowKeyboard("Repository URL", Anaconda.RepoUrl)
  if url and url ~= "" then
    Anaconda.RepoUrl = url
    Anaconda.SaveConfig()
    ShowMessage("Saved", "Repository URL updated.")
    Log("Repo URL changed to: " .. url)
  end
end

-- ============================================================
-- SECTION 6 : MAIN LOOP
-- ============================================================

function Anaconda.Main()
  Log("Anaconda XB 360 started")

  Anaconda.LoadConfig()

  local cats = Anaconda.LoadCategories()
  if not cats then
    ShowMessage(Anaconda.Name, "Could not reach repository:\n" .. Anaconda.RepoUrl)
    return
  end

  while true do
    local choice = Anaconda.CategoryMenu(cats)

    if not choice or choice == -1 then
      return
    elseif choice == 0 then
      Anaconda.Settings()
    else
      local cat = cats[choice]
      local pkgs = Anaconda.LoadPackages(cat.url)

      if not pkgs then
        ShowMessage(cat.name, "Failed to load category.")
      else
        local pick = Anaconda.PackageMenu(pkgs, cat.name)
        if pick and pick > 0 then
          local pkg = pkgs[pick]
          Anaconda.Install(pkg)
        end
      end
    end
  end
end

Anaconda.Main()
