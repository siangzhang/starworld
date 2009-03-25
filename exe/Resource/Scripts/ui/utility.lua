function class_attrib( item, name, indent )
	local attrib = item[name];
	if type( attrib ) == "userdata" then
		return attrib:members( indent + 1 );
	else
		return tostring( attrib );
	end;
end;

function save( item, name, node )
	local attrib = item[name];
	if type( attrib ) == "userdata" then
		local element = Element( name );
		attrib:save( element );
		node:InsertChild( element );
	else
		local element = Element( name );
		element:SetAttribute( "value", tostring( attrib ) );
		node:InsertChild( element );
	end;
end;

function load( item, name, node )
	if node then
		local attrib = item[name];
		if type( attrib ) == "userdata" then
			attrib:load( node );
		else
			local value = node:Attribute( "value" );
			item[name] = value;
		end;
	end
end;

-- cd ..\Resource\Scripts\ui\
-- open utility.lua
-- bp 2