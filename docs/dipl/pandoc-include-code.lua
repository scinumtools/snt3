-- Expand code blocks that refer to a shared fixture before Pandoc highlights
-- them. This keeps the rendered specification synchronized with the source
-- fixture used by the editor and Sphinx highlighters.
function CodeBlock(block)
  local include = block.attributes.include

  if include == nil then
    return nil
  end

  local file = assert(io.open(include, "r"), "Cannot read included code file: " .. include)
  block.text = file:read("*a")
  file:close()
  block.attributes.include = nil
  return block
end
