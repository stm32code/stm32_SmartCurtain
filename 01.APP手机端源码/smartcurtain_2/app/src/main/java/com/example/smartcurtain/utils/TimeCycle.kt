package com.example.smartcurtain.utils

import java.text.ParsePosition
import java.text.SimpleDateFormat
import java.time.format.DateTimeFormatter
import java.util.*

/**
 * 时间工具
 */
object TimeCycle {

    /**
     * 获取当前时间
     * return yyyy-MM-dd HH:mm:ss
     */
    fun getDateTime(): String {
        //转换为中国时区
        TimeZone.setDefault(TimeZone.getTimeZone("Etc/GMT-8"))
        return transToString(System.currentTimeMillis())
    }

    fun getDateTimeAndWeek(): String {
        //转换为中国时区
        TimeZone.setDefault(TimeZone.getTimeZone("Etc/GMT-8"))
        val calendar = Calendar.getInstance()

        // 获取当前日期和时间
        val year = calendar.get(Calendar.YEAR)
        val month = calendar.get(Calendar.MONTH) + 1
        val day = calendar.get(Calendar.DAY_OF_MONTH)
        val hour = calendar.get(Calendar.HOUR_OF_DAY)
        val minute = calendar.get(Calendar.MINUTE)
        val second = calendar.get(Calendar.SECOND)
        val dayOfWeek = calendar.get(Calendar.DAY_OF_WEEK)

        return String.format(
            "%04d%02d%02d%02d%02d%02d%02d",
            year,
            month,
            day,
            hour,
            minute,
            second,
            if (dayOfWeek == Calendar.SATURDAY) 6 else if (dayOfWeek == Calendar.SUNDAY) 7 else 1
        )
    }

    /**
     * 时间戳转字符串
     * @param Timestamp
     * @return String
     */
    private fun transToString(time: Long): String {
        val date = Date()
        val format = SimpleDateFormat("yyyy-MM-dd HH:mm:ss")
        return format.format(date)
//        return SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(time)
    }

    /**
     * 字符串转时间戳
     * @param String
     * @return Timestamp
     */
    fun transToTimeStamp(date: String): Long {
        return SimpleDateFormat("yyyy-MM-dd HH:mm:ss").parse(date, ParsePosition(0)).time
    }
}
