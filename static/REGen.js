/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

function selectForm(){
    if(document.forms['create-re']['re-type'][0].checked){
        $("#ints").removeClass("hide");
        $("#dates").addClass("hide");
        $(".floats-field").addClass("hide");
        document.getElementById("max-label").innerHTML = 'Max Digits: <span title="This field is required." class="form-required">*</span>';
        $("#phone-number").addClass("hide");
    }else if(document.forms['create-re']['re-type'][1].checked){
        $("#ints").addClass("hide");
        $("#dates").removeClass("hide");
        $("#phone-number").addClass("hide");
    }else if(document.forms['create-re']['re-type'][2].checked){
        $("#ints").removeClass("hide");
        $("#dates").addClass("hide");
        $(".floats-field").removeClass("hide");
        document.getElementById("max-label").innerHTML = 'Max Digits Before Decimal: <span title="This field is required." class="form-required">*</span>';
        $("#phone-number").addClass("hide");
    }else if(document.forms['create-re']['re-type'][3].checked){
        $("#ints").addClass('hide');
        $("#dates").addClass("hide");
        $("#phone-number").removeClass("hide");
    }
}

function createRegularExpression(){
    var expression = "";
    
    if(document.forms['create-re']['re-type'][0].checked){
        expression = createRegularExpressionInt();
    }else if(document.forms['create-re']['re-type'][1].checked){
        expression = createRegularExpressionDate();
    }else if(document.forms['create-re']['re-type'][2].checked){
        expression = createRegularExpressionFloat();
    }else if(document.forms['create-re']['re-type'][3].checked){
        expression = createRegularExpressionPhone();
    }
    
    document.getElementById("re").innerHTML = expression;
}

function createRegularExpressionDate(){
    var expression = "";
    var months = "(1[0-2]";
    var days = "([1-2][0-9]|3[0-1]";
    var years;
    var sep = "[";
    var customSeps = document.forms['create-re']['custom-sep-input'].value;
    
    if(!document.forms['create-re']['foreward-slash-sep'].checked
            && !document.forms['create-re']['period-sep'].checked
            && !document.forms['create-re']['custom-sep-check'].checked){
        expression = "Please select a separator.";
        return expression;
    }
    
    if(document.forms['create-re']['foreward-slash-sep'].checked){
        sep += "/";
    }
    
    if(document.forms['create-re']['period-sep'].checked){
        sep += ".";
    }
    
    if(document.forms['create-re']['custom-sep-check'].checked){
        for(var i = 0; i < customSeps.length; i++){
            if(customSeps[i] == '^' || customSeps[i] == '\\'){
                sep += "\\" + customSeps[i];
            }else{
                sep += customSeps[i];
            }
        }
    }
    
    sep += "]";
    
    if(document.forms['create-re']['year-selection'][0].checked){
        years = "([0-9]{2})";
    }else if(document.forms['create-re']['year-selection'][1].checked){
        years = "([0-9]{4})";
    }else{
        years = "([0-9]{2}|[0-9]{4})";
    }
    
    if(document.forms['create-re']['enforce-zero'].checked){
        months += "|0[0-9])";
        days += "|0[1-9])";
    }else{
        months += "|[0-9]|0[0-9])";
        days += "|[1-9]|0[1-9])";
    }
    
    if(document.forms['create-re']['edit-format'][0].checked){
        expression = months + sep + days + sep + years;//mdy
    }else{
        expression = days + sep + months + sep + years;//dmy
    }
    
    return expression;
}

function createRegularExpressionInt(){
    var maxDigits = document.forms["create-re"]["Max_dig"].value;
    var expression = "";
    var leadingZeroes = "";
    var unformattedMaxDigits = "[0-9]*";
    var formattedMaxDigitsOverflow = "[0-9]{0,2}";
    var formattedMaxDigitsGroups = "(,[0-9]{3})*";
    var commaSeparatorList = document.forms["create-re"]["comma-separator"];
    
    if((maxDigits != '' && isNaN(maxDigits))){
        document.getElementById("re").innerHTML = "Maximum Digits must be numerical.";//error
        return false;
    }
    
    
    
    if(document.forms["create-re"]["negative"].checked){
        if(document.forms['create-re']['neg-zero'].checked){
            expression += "((-?)0|(-?)";
        }else{
           expression += "(0|(-?)";
        }
    }else{
        expression += "(0|";
    }
    
    if(document.forms["create-re"]["leadingZeroes"].checked){
        leadingZeroes = "0";
    }else{
        leadingZeroes = '1';
    }
    
    if(maxDigits != '' && !isNaN(maxDigits)){
        maxDigits = parseInt(maxDigits) - 1;
        if(maxDigits !== 0){
            unformattedMaxDigits = "[0-9]{0," + maxDigits + "}"; //with limit
        }else{
            unformattedMaxDigits = "";
        }
        if(maxDigits % 3 > 0){
            formattedMaxDigitsOverflow = "[0-9]{0," + (maxDigits % 3) + "}";
        }else{
            formattedMaxDigitsOverflow = "";
        }
        if(parseInt(maxDigits / 3) > 0)
            formattedMaxDigitsGroups = "(,[0-9]{3}){0," + parseInt(maxDigits / 3) + "}";
        else
            formattedMaxDigitsGroups = "";
    } // if nothing, no limit
    
    if(commaSeparatorList[0].checked){  // YES
        expression += "[" + leadingZeroes + "-9]" + formattedMaxDigitsOverflow;
        expression += "" + formattedMaxDigitsGroups + ")";
    }else if(commaSeparatorList[1].checked){  //NO
        expression += "[" + leadingZeroes + "-9]" + unformattedMaxDigits + ")";
    }else{  //BOTH
        expression += "(([" + leadingZeroes + "-9]" + unformattedMaxDigits + ")|";
        expression += "([" + leadingZeroes + "-9]" + formattedMaxDigitsOverflow;
        expression += "" + formattedMaxDigitsGroups + ")))";
    }
    
    return expression;
    
}

function createRegularExpressionFloat(){
    var maxDigits = document.forms["create-re"]["Max_dig"].value;
    var maxPostDigits = document.forms['create-re']['Max_dig_post_dec'].value;
    var expression = "";
    var trailingZeroes = "";
    var maxDigitsPostDec = "[0-9]*";
    
    if((maxDigits != '' && isNaN(maxDigits)) || (maxPostDigits != '' && isNaN(maxPostDigits))){
        expression = "Maximum Digits Before and After Decimal must be numerical.";//error
        return expression;
    }
    
    expression = createRegularExpressionInt(); //builds pre-decimal portion
    
    if(document.forms['create-re']['trailingZeroes'].checked){
        trailingZeroes = "0"
    }else{
        trailingZeroes = '1';
    }
    
    if(maxPostDigits != '' && !isNaN(maxPostDigits)){
        maxPostDigits = parseInt(maxPostDigits) - 1;
        if(maxPostDigits !== 0){
            maxDigitsPostDec = "[0-9]{0," + maxPostDigits + "}";
        }else{
            maxDigitsPostDec = "";
        }
    }
    
    expression += "([.])(0|" + maxDigitsPostDec + "[" + trailingZeroes + "-9])";
    
    return expression;
    
}

function createRegularExpressionPhone(){
    var expression = "";
    var multipleFormats = false;
    var areaCode = document.forms["create-re"]["area-code"].value;
    
    if(!document.forms["create-re"]["full-format"].checked
            && !document.forms["create-re"]["dash-format"].checked
            && !document.forms["create-re"]["unformatted"].checked){
        expression = "You must select a format.";
        return expression;
    }
    
    if(areaCode.length != 3 || isNaN(areaCode)){
        areaCode = "[0-9]{3}";
    }
    
    if(document.forms['create-re']['full-format'].checked){
        multipleFormats = true;
        expression += "(([(]" + areaCode + "[)])([ ])([0-9]{3})([-])([0-9]{4}))";
    }
    
    if(document.forms["create-re"]["dash-format"].checked){
        if(multipleFormats){
            expression += "|";
        }else{
            multipleFormats = true;
        }
        expression += "((" + areaCode + ")([-])([0-9]{3})([-])([0-9]{4}))";
    }
    
    if(document.forms["create-re"]["unformatted"].checked){
        if(multipleFormats){
            expression += "|";
        }
        expression += "((" + areaCode + ")([0-9]{3})([0-9]{4}))";
    }
    return expression;
}



function swapNegZero(){
    if(document.forms["create-re"]["negative"].checked){
        $("#neg-zero-wrapper").removeClass('hide');//insert toggle class visible
    }else{
        $("#neg-zero-wrapper").addClass('hide');//insert toggle class invisible
    }
}
