#!/usr/bin/env ruby

require 'erubis'

puts "Content-type: text/html\n\n"

# Check that the script is being executed through a redirect
if ENV["REQUEST_URI"] =~ /^#{ENV["SCRIPT_NAME"]}.*/
  puts "<html><body><p>Script can not be executed directly!</p></body></html>"
else
  # Get the file location from the ENV hash, read it, and process it through erubis
  puts Erubis::FastEruby.new(File.read(ENV["PATH_TRANSLATED"])).result(binding())
end
