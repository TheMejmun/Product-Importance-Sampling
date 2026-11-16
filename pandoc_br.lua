function RawInline(el)
  if el.format:match('^html') and el.text:lower():match('^<br%s*/?>$') then
    return pandoc.LineBreak()
  end
end