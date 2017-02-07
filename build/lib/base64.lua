ZZBase64 = {}
local string = string

ZZBase64.__code = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
            'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
        };
ZZBase64.__decode = {}
for k,v in pairs(ZZBase64.__code) do
    ZZBase64.__decode[string.byte(v,1)] = k - 1
end

function ZZBase64.encode(text)
    local len = string.len(text)
    local left = len % 3
    len = len - left
    local res = {}
    local index  = 1
    for i = 1, len, 3 do
        local a = string.byte(text, i )
        local b = string.byte(text, i + 1)
        local c = string.byte(text, i + 2)
        -- num = a<<16 + b<<8 + c
        local num = a * 65536 + b * 256 + c 
        for j = 1, 4 do
            --tmp = num >> ((4 -j) * 6)
            local tmp = math.floor(num / (2 ^ ((4-j) * 6)))
            --curPos = tmp&0x3f
            local curPos = tmp % 64 + 1
            res[index] = ZZBase64.__code[curPos]
            index = index + 1
        end
    end

    if left == 1 then
        ZZBase64.__left1(res, index, text, len)
    elseif left == 2 then
        ZZBase64.__left2(res, index, text, len)        
    end
    return table.concat(res)
end

function ZZBase64.__left2(res, index, text, len)
    local num1 = string.byte(text, len + 1)
    num1 = num1 * 1024 --lshift 10 
    local num2 = string.byte(text, len + 2)
    num2 = num2 * 4 --lshift 2 
    local num = num1 + num2
   
    local tmp1 = math.floor(num / 4096) --rShift 12
    local curPos = tmp1 % 64 + 1
    res[index] = ZZBase64.__code[curPos]
    
    local tmp2 = math.floor(num / 64)
    curPos = tmp2 % 64 + 1
    res[index + 1] = ZZBase64.__code[curPos]

    curPos = num % 64 + 1
    res[index + 2] = ZZBase64.__code[curPos]
    
    res[index + 3] = "=" 
end

function ZZBase64.__left1(res, index,text, len)
    local num = string.byte(text, len + 1)
    num = num * 16 
    
    tmp = math.floor(num / 64)
    local curPos = tmp % 64 + 1
    res[index ] = ZZBase64.__code[curPos]
    
    curPos = num % 64 + 1
    res[index + 1] = ZZBase64.__code[curPos]
    
    res[index + 2] = "=" 
    res[index + 3] = "=" 
end

function ZZBase64.decode(text)
    local len = string.len(text)
    local left = 0 
    if string.sub(text, len - 1) == "==" then
        left = 2 
        len = len - 4
    elseif string.sub(text, len) == "=" then
        left = 1
        len = len - 4
    end

    local res = {}
    local index = 1
    local decode = ZZBase64.__decode
    for i =1, len, 4 do
        local a = decode[string.byte(text,i    )] 
        local b = decode[string.byte(text,i + 1)] 
        local c = decode[string.byte(text,i + 2)] 
        local d = decode[string.byte(text,i + 3)]

        --num = a<<18 + b<<12 + c<<6 + d
        local num = a * 262144 + b * 4096 + c * 64 + d
        
        local e = string.char(num % 256)
        num = math.floor(num / 256)
        local f = string.char(num % 256)
        num = math.floor(num / 256)
        res[index ] = string.char(num % 256)
        res[index + 1] = f
        res[index + 2] = e
        index = index + 3
    end

    if left == 1 then
        ZZBase64.__decodeLeft1(res, index, text, len)
    elseif left == 2 then
        ZZBase64.__decodeLeft2(res, index, text, len)
    end
    return table.concat(res)
end

function ZZBase64.__decodeLeft1(res, index, text, len)
    local decode = ZZBase64.__decode
    local a = decode[string.byte(text, len + 1)] 
    local b = decode[string.byte(text, len + 2)] 
    local c = decode[string.byte(text, len + 3)] 
    local num = a * 4096 + b * 64 + c
    
    local num1 = math.floor(num / 1024) % 256
    local num2 = math.floor(num / 4) % 256
    res[index] = string.char(num1)
    res[index + 1] = string.char(num2)
end

function ZZBase64.__decodeLeft2(res, index, text, len)
    local decode = ZZBase64.__decode
    local a = decode[string.byte(text, len + 1)] 
    local b = decode[string.byte(text, len + 2)]
    local num = a * 64 + b
    num = math.floor(num / 16)
    res[index] = string.char(num)
end